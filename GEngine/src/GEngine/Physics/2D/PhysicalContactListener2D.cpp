#include "GEpch.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"
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
    // Standalone listeners may override these hooks; scene script dispatch is removed.
    void PhysicalContactListener2D::OnContactBegin(Ref<Physics2DContactInfo> info) {}
    void PhysicalContactListener2D::OnContactEnd(Ref<Physics2DContactInfo> info) {}
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
