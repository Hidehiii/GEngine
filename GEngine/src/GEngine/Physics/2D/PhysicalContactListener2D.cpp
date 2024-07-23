#include "GEpch.h"
#include "PhysicalContactListener2D.h"
#include "GEngine/Physics/2D/Physics2D.h"
#include "GEngine/Object/Scene/Scene.h"

#include <box2d/box2d.h>

namespace GEngine
{
    void PhysicalContactListener2D::BeginContact(b2Contact* contact)
    {
        Ref<Physics2DContactInfo> contactInfo = CreateRef<Physics2DContactInfo>();
        contactInfo->m_Contact = contact;
        contactInfo->m_Scene = this->m_Scene;
        this->OnContactBegin(contactInfo);
    }
    void PhysicalContactListener2D::EndContact(b2Contact* contact)
    {
		Ref<Physics2DContactInfo> contactInfo = CreateRef<Physics2DContactInfo>();
		contactInfo->m_Contact = contact;
        contactInfo->m_Scene = this->m_Scene;
		this->OnContactEnd(contactInfo);
    }
    void PhysicalContactListener2D::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
    {
    }
    void PhysicalContactListener2D::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
    {
    }
    // 碰撞体开始接触
    void PhysicalContactListener2D::OnContactBegin(Ref<Physics2DContactInfo> info)
    {
        GameObject gameObjectA = GameObject{ (uint32_t)info->m_Contact->GetFixtureA()->GetUserData().pointer, this->m_Scene };
        GameObject gameObjectB = GameObject{ (uint32_t)info->m_Contact->GetFixtureB()->GetUserData().pointer, this->m_Scene };

        auto scriptA = gameObjectA.TryGetComponent<NativeScript>();
        if (scriptA)
        {
            Ref<Physics2DContactInfo> info_A = CreateRef<Physics2DContactInfo>();
            info_A->m_Contact = info->m_Contact;
            info_A->m_Scene = info->m_Scene;
            info_A->m_OtherGameObject = gameObjectB;
            scriptA->Instance->OnCollisionEnter2D(info_A);
        }

        auto scriptB = gameObjectB.TryGetComponent<NativeScript>();
        if (scriptB)
        {
            Ref<Physics2DContactInfo> info_B = CreateRef<Physics2DContactInfo>();
            info_B->m_Contact = info->m_Contact;
            info_B->m_Scene = info->m_Scene;
            info_B->m_OtherGameObject = gameObjectA;
            scriptB->Instance->OnCollisionEnter2D(info_B);
        }
    }
    // 碰撞体结束接触
    void PhysicalContactListener2D::OnContactEnd(Ref<Physics2DContactInfo> info)
    {
        GameObject gameObjectA = GameObject{ (uint32_t)info->m_Contact->GetFixtureA()->GetUserData().pointer, this->m_Scene };
		GameObject gameObjectB = GameObject{ (uint32_t)info->m_Contact->GetFixtureB()->GetUserData().pointer, this->m_Scene };

		auto scriptA = gameObjectA.TryGetComponent<NativeScript>();
		if (scriptA)
		{
			scriptA->Instance->OnCollisionExit2D(info);
		}

		auto scriptB = gameObjectB.TryGetComponent<NativeScript>();
		if (scriptB)
		{
			scriptB->Instance->OnCollisionExit2D(info);
		}
    }
    GameObject Physics2DContactInfo::GetOtherGameObject()
    {
        return m_OtherGameObject;
    }
    Ref<Physics2DFixture> Physics2DContactInfo::GetFixtureA()
    {
        Ref<Physics2DFixture> fixture = CreateRef<Physics2DFixture>();
        fixture->m_GameObject = GameObject{ (uint32_t)m_Contact->GetFixtureA()->GetUserData().pointer, this->m_Scene };
        fixture->m_Fixture = m_Contact->GetFixtureA();
        return fixture;
    }
    Ref<Physics2DFixture> Physics2DContactInfo::GetFixtureB()
    {
        Ref<Physics2DFixture> fixture = CreateRef<Physics2DFixture>();
        fixture->m_GameObject = GameObject{ (uint32_t)m_Contact->GetFixtureB()->GetUserData().pointer, this->m_Scene };
        fixture->m_Fixture = m_Contact->GetFixtureB();
        return fixture;
    }
}