#include <cyclone/cyclone.h>
#include "../ogl_headers.h"
#include "../app.h"
#include "../timing.h"

#include <stdio.h>
#include <iostream>


class SpringLimitDemo : public Application
{
   cyclone::Spring spring;
   cyclone::RigidBody other;
   cyclone::ForceRegistry registry;
   
   float springConst = 5.0f;
   float restLength = 10.0f;

public:
   SpringLimitDemo();
   virtual ~SpringLimitDemo();

   virtual void update();
};

SpringLimitDemo::SpringLimitDemo()
:
Application(),
spring(cyclone::Vector3(0,0,0), &other, cyclone::Vector3(20.0f,0,0), springConst, restLength)
{

   other.setPosition(20.0f,0,0);
   other.setOrientation(1,0,0,0);
   
   other.setVelocity(0,0,0);
   other.setMass(1.0f);
   other.setDamping(0.75f, 0.75f);
   other.setAcceleration(0,0,0);

   registry.add(&other, &spring);
}

SpringLimitDemo::~SpringLimitDemo()
{
}

void SpringLimitDemo::update()
{
   // Find the duration of the last frame in seconds
   float duration = 0.02f;
   if (duration <= 0.0f) return;

   // Start with no forces or acceleration.
   other.clearAccumulators();

   registry.updateForces(duration);

   // Update the boat's physics.
   other.integrate(duration);

   std::cout << other.getPosition().x << std::endl;

   Application::update();
}

Application* getApplication()
{
   return new SpringLimitDemo();
}