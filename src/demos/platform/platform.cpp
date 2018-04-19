/*
 * The platform demo.
 *
 * Part of the Cyclone physics system.
 *
 * Copyright (c) Icosagon 2003. All Rights Reserved.
 *
 * This software is distributed under licence. Use of this software
 * implies agreement with all terms and conditions of the accompanying
 * software licence.
 */

#include <cyclone/cyclone.h>
#include "../ogl_headers.h"
#include "../app.h"
#include "../timing.h"

#include <stdio.h>
#include <cassert>
#include <cmath>

#define ROD_COUNT 24

#define BASE_MASS 1
#define EXTRA_MASS 100

#define PARTICLE_COUNT 8

/**
 * The main demo class definition.
 */
class PlatformDemo : public MassAggregateApplication
{
    cyclone::ParticleRod *rods;

    cyclone::Vector3 massPos;
    cyclone::Vector3 massDisplayPos;

    /**
     * Updates particle masses to take into account the mass
     * that's on the platform.
     */
    void updateAdditionalMass();

public:
    /** Creates a new demo object. */
    PlatformDemo();
    virtual ~PlatformDemo();

    /** Returns the window title for the demo. */
    virtual const char* getTitle();

    /** Display the particles. */
    virtual void display();

    /** Update the particle positions. */
    virtual void update();

    /** Handle a key press. */
    virtual void key(unsigned char key);
};

// Method definitions
PlatformDemo::PlatformDemo()
:
MassAggregateApplication(PARTICLE_COUNT), rods(0),
massPos(0,0,0.5f)
{
    int rodLength = 5;

    // Create the masses and connections.
    particleArray[0].setPosition(0,0,rodLength);
    particleArray[1].setPosition(0,0,-rodLength);
    particleArray[2].setPosition(rodLength,0,rodLength);
    particleArray[3].setPosition(rodLength,0,-rodLength);
    particleArray[4].setPosition(0,rodLength,rodLength);
    particleArray[5].setPosition(0,rodLength,-rodLength);
    particleArray[6].setPosition(rodLength,rodLength,rodLength);
    particleArray[7].setPosition(rodLength,rodLength,-rodLength);
    for (unsigned i = 0; i < PARTICLE_COUNT; i++)
    {
        particleArray[i].setMass(BASE_MASS);
        particleArray[i].setVelocity(0,0,0);
        particleArray[i].setDamping(0.9f);
        particleArray[i].setAcceleration(cyclone::Vector3::GRAVITY);
        particleArray[i].clearAccumulator();
    }

    rods = new cyclone::ParticleRod[ROD_COUNT];

    // edges
    rods[0].particle[0] = &particleArray[0];
    rods[0].particle[1] = &particleArray[1];
    rods[0].length = rodLength;
    rods[1].particle[0] = &particleArray[0];
    rods[1].particle[1] = &particleArray[2];
    rods[1].length = rodLength;

    rods[2].particle[0] = &particleArray[2];
    rods[2].particle[1] = &particleArray[3];
    rods[2].length = rodLength;
    rods[3].particle[0] = &particleArray[3];
    rods[3].particle[1] = &particleArray[1];
    rods[3].length = rodLength;

    rods[4].particle[0] = &particleArray[0];
    rods[4].particle[1] = &particleArray[4];
    rods[4].length = rodLength;
    rods[5].particle[0] = &particleArray[2];
    rods[5].particle[1] = &particleArray[6];
    rods[5].length = rodLength;

    rods[6].particle[0] = &particleArray[1];
    rods[6].particle[1] = &particleArray[5];
    rods[6].length = rodLength;
    rods[7].particle[0] = &particleArray[3];
    rods[7].particle[1] = &particleArray[7];
    rods[7].length = rodLength;

    rods[8].particle[0] = &particleArray[4];
    rods[8].particle[1] = &particleArray[6];
    rods[8].length = rodLength;
    rods[9].particle[0] = &particleArray[6];
    rods[9].particle[1] = &particleArray[7];
    rods[9].length = rodLength;

    rods[10].particle[0] = &particleArray[7];
    rods[10].particle[1] = &particleArray[5];
    rods[10].length = rodLength;
    rods[11].particle[0] = &particleArray[5];
    rods[11].particle[1] = &particleArray[4];
    rods[11].length = rodLength;

    // diagonal
    double diagLength = 7.071;

    rods[12].particle[0] = &particleArray[0];
    rods[12].particle[1] = &particleArray[6];
    rods[12].length = diagLength;
    rods[13].particle[0] = &particleArray[4];
    rods[13].particle[1] = &particleArray[2];
    rods[13].length = diagLength;
    rods[14].particle[0] = &particleArray[0];
    rods[14].particle[1] = &particleArray[5];
    rods[14].length = diagLength;
    rods[15].particle[0] = &particleArray[1];
    rods[15].particle[1] = &particleArray[4];
    rods[15].length = diagLength;

    rods[16].particle[0] = &particleArray[1];
    rods[16].particle[1] = &particleArray[7];
    rods[16].length = diagLength;
    rods[17].particle[0] = &particleArray[3];
    rods[17].particle[1] = &particleArray[5];
    rods[17].length = diagLength;

    rods[18].particle[0] = &particleArray[3];
    rods[18].particle[1] = &particleArray[6];
    rods[18].length = diagLength;
    rods[19].particle[0] = &particleArray[2];
    rods[19].particle[1] = &particleArray[7];
    rods[19].length = diagLength;

    rods[20].particle[0] = &particleArray[4];
    rods[20].particle[1] = &particleArray[7];
    rods[20].length = diagLength;
    rods[21].particle[0] = &particleArray[5];
    rods[21].particle[1] = &particleArray[6];
    rods[21].length = diagLength;

    rods[22].particle[0] = &particleArray[0];
    rods[22].particle[1] = &particleArray[3];
    rods[22].length = diagLength;
    rods[23].particle[0] = &particleArray[1];
    rods[23].particle[1] = &particleArray[2];
    rods[23].length = diagLength;
    /*
    rods[14].particle[0] = &particleArray[7];
    rods[14].particle[1] = &particleArray[5];
    rods[14].length = rodLength;
    rods[15].particle[0] = &particleArray[5];
    rods[15].particle[1] = &particleArray[4];
    rods[15].length = rodLength;
    */

    

    for (unsigned i = 0; i < ROD_COUNT; i++)
    {
        world.getContactGenerators().push_back(&rods[i]);
    }

    updateAdditionalMass();
}

PlatformDemo::~PlatformDemo()
{
    if (rods) delete[] rods;
}

void PlatformDemo::updateAdditionalMass()
{
    for (unsigned i = 0; i < PARTICLE_COUNT; i++)
    {
        particleArray[i].setMass(BASE_MASS);
    }

    // Find the coordinates of the mass as an index and proportion
    cyclone::real xp = massPos.x;
    if (xp < 0) xp = 0;
    if (xp > 1) xp = 1;

    cyclone::real zp = massPos.z;
    if (zp < 0) zp = 0;
    if (zp > 1) zp = 1;

    // Calculate where to draw the mass
    massDisplayPos.clear();

    // Add the proportion to the correct masses
    particleArray[2].setMass(BASE_MASS + EXTRA_MASS*(1-xp)*(1-zp));
    massDisplayPos.addScaledVector(
        particleArray[2].getPosition(), (1-xp)*(1-zp)
        );

    if (xp > 0)
    {
        particleArray[4].setMass(BASE_MASS + EXTRA_MASS*xp*(1-zp));
        massDisplayPos.addScaledVector(
            particleArray[4].getPosition(), xp*(1-zp)
            );

        if (zp > 0)
        {
            particleArray[5].setMass(BASE_MASS + EXTRA_MASS*xp*zp);
            massDisplayPos.addScaledVector(
                particleArray[5].getPosition(), xp*zp
                );
        }
    }
    if (zp > 0)
    {
        particleArray[3].setMass(BASE_MASS + EXTRA_MASS*(1-xp)*zp);
        massDisplayPos.addScaledVector(
            particleArray[3].getPosition(), (1-xp)*zp
            );
    }
}

void PlatformDemo::display()
{
    MassAggregateApplication::display();

    glBegin(GL_LINES);
    glColor3f(0,0,1);
    for (unsigned i = 0; i < ROD_COUNT; i++)
    {
        cyclone::Particle **particles = rods[i].particle;
        const cyclone::Vector3 &p0 = particles[0]->getPosition();
        const cyclone::Vector3 &p1 = particles[1]->getPosition();
        glVertex3f(p0.x, p0.y, p0.z);
        glVertex3f(p1.x, p1.y, p1.z);
    }
    glEnd();

    glColor3f(1,0,0);
    glPushMatrix();
    glTranslatef(massDisplayPos.x, massDisplayPos.y+0.25f, massDisplayPos.z);
    glutSolidSphere(0.25f, 20, 10);
    glPopMatrix();
}

void PlatformDemo::update()
{
    MassAggregateApplication::update();

    updateAdditionalMass();
}

const char* PlatformDemo::getTitle()
{
    return "Cyclone > Platform Demo";
}

void PlatformDemo::key(unsigned char key)
{
    switch(key)
    {
    case 'w': case 'W':
        massPos.z += 0.1f;
        if (massPos.z > 1.0f) massPos.z = 1.0f;
        break;
    case 's': case 'S':
        massPos.z -= 0.1f;
        if (massPos.z < 0.0f) massPos.z = 0.0f;
        break;
    case 'a': case 'A':
        massPos.x -= 0.1f;
        if (massPos.x < 0.0f) massPos.x = 0.0f;
        break;
    case 'd': case 'D':
        massPos.x += 0.1f;
        if (massPos.x > 1.0f) massPos.x = 1.0f;
        break;

    default:
        MassAggregateApplication::key(key);
    }
}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new PlatformDemo();
}