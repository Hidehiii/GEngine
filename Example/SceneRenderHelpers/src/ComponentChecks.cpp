#include <GEngine.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace GEngine
{
    void RunComponentChecks()
    {
        static_assert(!IsSceneComponent<Transform> && !IsSceneComponent<GameObjectData>);
        static_assert(IsSceneComponent<StaticMeshComponent> && IsSceneComponent<ImageComponent> &&
            IsSceneComponent<CameraComponent> && IsSceneComponent<DirectionalLightComponent>);
        auto require = [](bool value, const char* message)
        { if (!value) throw std::runtime_error(message); };
        auto reject = [&](auto operation, const char* message)
        {
            bool rejected = false;
            try { operation(); } catch (const std::exception&) { rejected = true; }
            require(rejected, message);
        };
        auto scene = CreateRef<Scene>();
        auto parent = scene->CreateGameObject(0x100000001ull, "Parent");
        auto child = scene->CreateGameObject(0x200000001ull, "Child");
        child.SetParent(parent);
        child.SetActive(false);
        child.GetTransform().SetPosition({1, 2, 3});
        parent.AddComponent<CameraComponent>(CameraType::Perspective);
        parent.GetComponent<CameraComponent>().SetPerspective(55, 0.1f, 500);
        parent.GetComponent<CameraComponent>().isPrimary = true;
        scene->OnViewportResize(800, 400);
        require(scene->MainCamera()->GetAspectRatio() == 2, "CameraComponent viewport behavior changed.");
        reject([&] { parent.SetParent(child); }, "Hierarchy cycle was accepted.");
        reject([&] { scene->CreateGameObject(0x100000001ull, "Duplicate"); }, "Duplicate UUID was accepted.");
        parent.AddComponent<ImageComponent>();
        auto& image = parent.GetComponent<ImageComponent>();
        image.LoadImageAsset("Assets/Checker.gasset");
        image.SetMaterial(Material::Create("Assets/Shaders/ImageColor.gasset", "Persisted image"));
        image.Color = {0.2f, 0.3f, 0.4f, 1};
        image.Tiling = {2, 3};
        parent.AddComponent<DirectionalLightComponent>();
        auto& light = parent.GetComponent<DirectionalLightComponent>();
        light.Color = {0.9f, 0.85f, 0.75f, 1.0f};
        light.Intensity = 1.25f;
        require(light.GetDirection() == Vector3(0.0f, 0.0f, -1.0f), "DirectionalLightComponent default direction differs.");
        child.AddComponent<StaticMeshComponent>();
        auto& mesh = child.GetComponent<StaticMeshComponent>();
        mesh.LoadMeshAsset("Assets/Mesh.gasset");
        mesh.SetMaterial(Material::Create("Assets/Shaders/MeshColor.gasset", "Persisted mesh"));
        auto originalMesh = mesh.GetMeshAsset();
        auto invalid = CreateRef<Mesh>(*originalMesh);
        invalid->m_Indices.front() = static_cast<uint32_t>(invalid->m_Vertices.size());
        reject([&] { mesh.SetMeshAsset(invalid); }, "Invalid mesh index was accepted.");
        require(mesh.GetMeshAsset() == originalMesh, "Failed asset replacement changed the mesh.");
        auto copy = Scene::Copy(scene);
        auto copiedChild = copy->GetGameObjects<StaticMeshComponent>().front();
        auto copiedParent = copiedChild.Parent();
        require(copiedParent && copiedParent != parent && copiedParent.GetUUID() == parent.GetUUID(), "Copied hierarchy retains source handles.");
        require(copiedChild.GetTransform().GetPosition() == Vector3(1, 2, 3) && !copiedChild.IsActive(), "Copied base data differs.");
        require(copiedChild.GetComponent<StaticMeshComponent>().GetMeshAsset() == originalMesh, "Scene copy did not share mesh asset ownership.");
        require(copiedParent.GetComponent<ImageComponent>().GetImageAsset() == image.GetImageAsset(), "Scene copy did not share image ownership.");
        require(copiedParent.GetComponent<DirectionalLightComponent>().Color == light.Color &&
            copiedParent.GetComponent<DirectionalLightComponent>().Intensity == light.Intensity, "Copied DirectionalLight data differs.");
        require(copiedChild.GetComponent<StaticMeshComponent>().GetMaterial() == mesh.GetMaterial(), "Scene copy did not retain its material.");
        scene->DuplicateGameObject(child);
        require(scene->GetGameObjects<StaticMeshComponent>().size() == 2, "Object duplication failed.");
        for (auto object : scene->GetGameObjects<StaticMeshComponent>())
            require(object.GetName() == "Child" && object.Parent() == parent && !object.IsActive(), "Duplicated base data differs.");
        const auto temporary = std::filesystem::temp_directory_path() /
            ("gengine-components-" + std::to_string(static_cast<uint64_t>(UUID())) + ".gscene");
        const auto malformed = temporary.parent_path() / (temporary.stem().string() + ".gasset");
        auto cleanup = [&]
        {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            std::filesystem::remove(malformed, ignored);
        };
        try
        {
            Serializer::Serialize(temporary.u8string(), copy);
            Ref<Scene> restored = CreateRef<Scene>();
            Serializer::Deserialize(temporary.u8string(), restored);
            auto loaded = restored->GetGameObjects<StaticMeshComponent>().front();
            require(static_cast<uint64_t>(loaded.GetUUID()) == 0x200000001ull && !loaded.IsActive(), "Scene UUID/active roundtrip failed.");
            require(loaded.Parent().GetName() == "Parent" && loaded.GetComponent<StaticMeshComponent>().GetMeshAsset(), "Scene hierarchy/mesh roundtrip failed.");
            auto& loadedImage = loaded.Parent().GetComponent<ImageComponent>();
            require(loadedImage.GetImageAsset() && loadedImage.Color == image.Color && loadedImage.Tiling == image.Tiling,
                "ImageComponent asset/color/tiling roundtrip failed.");
            require(loadedImage.GetMaterial()->GetName() == "Persisted image" && restored->MainCamera()->GetPerspectiveVerticalFOV() == 55,
                "Material/camera roundtrip failed.");
            auto& loadedLight = loaded.Parent().GetComponent<DirectionalLightComponent>();
            require(loadedLight.Color == light.Color && loadedLight.Intensity == light.Intensity,
                "DirectionalLight color/intensity roundtrip failed.");
            const auto before = restored;
            {
                std::ofstream file(temporary);
                file << "Scene: Removed\nGameObjects:\n  - GameObject: 1\n    RigidBody2D: {}\n";
                file.close(); require(static_cast<bool>(file), "Cannot write removed-component fixture.");
            }
            reject([&] { Serializer::Deserialize(temporary.u8string(), restored); }, "Removed component was silently ignored.");
            require(restored == before, "Failed scene load replaced the destination.");
            {
                std::ofstream file(malformed);
                file << "schemaVersion: 1\ntype: mesh\nformat: fbx\npayloadEncoding: text\npayload: broken\n";
                file.close(); require(static_cast<bool>(file), "Cannot write invalid mesh fixture.");
            }
            reject([&] { mesh.LoadMeshAsset(malformed.u8string()); }, "Invalid FBX was accepted.");
            require(mesh.GetMeshAsset() == originalMesh, "Failed mesh load changed its owner.");
        }
        catch (...) { cleanup(); throw; }
        cleanup();
        scene->DestroyGameObject(parent);
        scene->DestroyGameObject(parent);
        scene->OnEndFrame();
        require(!parent && !child.Parent(), "Deletion did not detach hierarchy safely.");
        std::weak_ptr<const Mesh> owner = copiedChild.GetComponent<StaticMeshComponent>().GetMeshAsset();
        originalMesh.reset();
        scene.reset();
        require(!owner.expired() && copiedChild.GetComponent<StaticMeshComponent>().GetMeshAsset(), "Copied assets were destroyed with the source scene.");
        GE_INFO("Component checks passed: base data, CameraComponent, asset/material ownership, copy/duplicate, scene roundtrip and rejection paths.");
    }
}
