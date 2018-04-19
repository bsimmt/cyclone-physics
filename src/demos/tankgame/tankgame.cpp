 /*
 * The BigBallistic demo.
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
#include <iostream>

enum ShotType
{
	UNUSED = 0,
	PISTOL,
	ARTILLERY,
	FIREBALL,
	LASER
};

class AmmoRound : public cyclone::CollisionSphere
{
public:
	ShotType type;
	unsigned startTime;

	AmmoRound()
	{
		body = new cyclone::RigidBody;
	}

	~AmmoRound()
	{
		delete body;
	}

	/** Draws the box, excluding its shadow. */
	void render()
	{
		// Get the OpenGL transformation
		GLfloat mat[16];
		body->getGLTransform(mat);

		glPushMatrix();
		glMultMatrixf(mat);
		glutSolidSphere(radius, 20, 20);
		glPopMatrix();
	}

	/** Sets the box to a specific location. */
	void setState(ShotType shotType)
	{
		type = shotType;

		// Set the properties of the particle
		switch(type)
		{
		case PISTOL:
			body->setMass(1.5f);
			body->setVelocity(0.0f, 0.0f, 20.0f);
			body->setAcceleration(0.0f, -0.5f, 0.0f);
			body->setDamping(0.99f, 0.8f);
			radius = 0.2f;
			break;

		case ARTILLERY:
			body->setMass(2000.0f); // 200.0kg
			body->setVelocity(0.0f, 60.0f, 40.0f); // 50m/s
			body->setAcceleration(0.0f, -21.0f, 0.0f);
			body->setDamping(0.99f, 0.8f);
			radius = 2.0f;
			break;

		case FIREBALL:
			body->setMass(4.0f); // 4.0kg - mostly blast damage
			body->setVelocity(0.0f, -0.5f, 10.0); // 10m/s
			body->setAcceleration(0.0f, 0.3f, 0.0f); // Floats up
			body->setDamping(0.9f, 0.8f);
			radius = 0.6f;
			break;

		case LASER:
			// Note that this is the kind of laser bolt seen in films,
			// not a realistic laser beam!
			body->setMass(0.1f); // 0.1kg - almost no weight
			body->setVelocity(0.0f, 0.0f, 100.0f); // 100m/s
			body->setAcceleration(0.0f, 0.0f, 0.0f); // No gravity
			body->setDamping(0.99f, 0.8f);
			radius = 0.2f;
			break;
		}

		body->setCanSleep(false);
		body->setAwake();

		cyclone::Matrix3 tensor;
		cyclone::real coeff = 0.4f*body->getMass()*radius*radius;
		tensor.setInertiaTensorCoeffs(coeff,coeff,coeff);
		body->setInertiaTensor(tensor);

		// Set the data common to all particle types
		body->setPosition(0.0f, 1.5f, 0.0f);
		startTime = TimingData::get().lastFrameTimestamp;

		// Clear the force accumulators
		body->calculateDerivedData();
		calculateInternals();
	}
};

class Box : public cyclone::CollisionBox
{
public:
	bool isOverlapping;

    Box()
    {
        body = new cyclone::RigidBody;
    }

    ~Box()
    {
        delete body;
    }

    /** Draws the box, excluding its shadow. */
    void render()
    {
        // Get the OpenGL transformation
        GLfloat mat[16];
        body->getGLTransform(mat);

        if (isOverlapping) glColor3f(0.7f,1.0f,0.7f);
        else if (body->getAwake()) glColor3f(1.0f,0.7f,0.7f);
        else glColor3f(0.7f,0.7f,1.0f);

        glPushMatrix();
        glMultMatrixf(mat);
        glScalef(halfSize.x*2, halfSize.y*2, halfSize.z*2);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    /** Draws the ground plane shadow for the box. */
    void renderShadow()
    {
        // Get the OpenGL transformation
        GLfloat mat[16];
        body->getGLTransform(mat);

        glPushMatrix();
        glScalef(1.0f, 0, 1.0f);
        glMultMatrixf(mat);
        glScalef(halfSize.x*2, halfSize.y*2, halfSize.z*2);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

	/** Sets the box to a specific location. */
	void setState(cyclone::real y, cyclone::real z)
	{
		body->setPosition(0, y, z);
		body->setOrientation(1,0,0,0);
		body->setVelocity(0,0,0);
		body->setRotation(cyclone::Vector3(0,0,0));
		halfSize = cyclone::Vector3(5,5,5);

		cyclone::real mass = halfSize.x * halfSize.y * halfSize.z * 1.0f;
		body->setMass(mass);

		cyclone::Matrix3 tensor;
		tensor.setBlockInertiaTensor(halfSize, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.8f);
		body->setAngularDamping(0.25f);
		body->clearAccumulators();
		body->setAcceleration(0,-10.0f,0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();
		calculateInternals();
	}
};


/**
 * The main demo class definition.
 */
class BigBallisticDemo : public RigidBodyApplication
{
	/**
	 * Holds the maximum number of  rounds that can be
	 * fired.
	 */
	const static unsigned ammoRounds = 256;

	/** Holds the particle data. */
	AmmoRound ammo[ammoRounds];

	/**
	* Holds the number of boxes in the simulation.
	*/
	const static unsigned boxes = 20;

	/** Holds the box data. */
	Box boxData[boxes];

	/** Holds the current shot type. */
	ShotType currentShotType;

	/** Resets the position of all the boxes and primes the explosion. */
	virtual void reset();

	/** Build the contacts for the current situation. */
	virtual void generateContacts();

	/** Processes the objects in the simulation forward in time. */
	virtual void updateObjects(cyclone::real duration);

	/** Dispatches a round. */
	void fire();

	// gluLookAt values
	// default: gluLookAt(-25.0, 8.0, 5.0,  0.0, 5.0, 22.0,  0.0, 1.0, 0.0);
	double x1 = -90.0;
	double y1 = 25.0;
	double z1 = 65.0;

	double x2 = 0.0;
	double y2 = 35.0;
	double z2 = 65.0;

	// don't change
	double x3 = 0.0;
	double y3 = 1.0;
	double z3 = 0.0;



public:
	/** Creates a new demo object. */
	BigBallisticDemo();

	/** Returns the window title for the demo. */
	virtual const char* getTitle();

	/** Sets up the rendering. */
	virtual void initGraphics();

	/** Display world. */
	virtual void display();

	/** Handle a mouse click. */
	virtual void mouse(int button, int state, int x, int y);

	/** Handle a keypress. */
	virtual void key(unsigned char key);
};

// Method definitions
BigBallisticDemo::BigBallisticDemo()
:
RigidBodyApplication(),
currentShotType(LASER)
{
	pauseSimulation = false;
	reset();
}


void BigBallisticDemo::initGraphics()
{
	GLfloat lightAmbient[] = {0.8f,0.8f,0.8f,1.0f};
	GLfloat lightDiffuse[] = {0.9f,0.95f,1.0f,1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

	glEnable(GL_LIGHT0);

	Application::initGraphics();
}

void BigBallisticDemo::reset()
{
	// Make all shots unused
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		shot->type = UNUSED;
	}

	// Initialise the box
	cyclone::real z = 20.0f;
	cyclone::real y = 0.0f;
	int i = 0;
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		box->setState(y, z);
		z += 11.0f;
		i++;
		if(i%5 == 0) {
			z = 20.0f;
			y += 11.0f;
		}	
	}
}

const char* BigBallisticDemo::getTitle()
{
	return "Cyclone > Big Ballistic Demo";
}

void BigBallisticDemo::fire()
{
	// Find the first available round.
	AmmoRound *shot;
	for (shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type == UNUSED) break;
	}

	// If we didn't find a round, then exit - we can't fire.
	if (shot >= ammo+ammoRounds) return;

	// Set the shot
	shot->setState(currentShotType);

}

void BigBallisticDemo::updateObjects(cyclone::real duration)
{
	// Update the physics of each particle in turn
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type != UNUSED)
		{
			// Run the physics
			shot->body->integrate(duration);
			shot->calculateInternals();

			// Check if the particle is now invalid
			if (shot->body->getPosition().y < 0.0f ||
				shot->startTime+5000 < TimingData::get().lastFrameTimestamp ||
				shot->body->getPosition().z > 200.0f)
			{
				// We simply set the shot type to be unused, so the
				// memory it occupies can be reused by another shot.
				shot->type = UNUSED;
			}
		}
	}

	// Update the boxes
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		// Run the physics
		box->body->integrate(duration);
		box->calculateInternals();
		box->isOverlapping = false;
	}


}

void BigBallisticDemo::display()
{
	const static GLfloat lightPosition[] = {-1,1,0,0};

	// Clear the viewport and set the camera direction
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(x1, y1, z1,  x2, y2, z2,  x3, y3, z3);

	// Draw a sphere at the firing point, and add a shadow projected
	// onto the ground plane.
	glColor3f(0.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, 0.0f);
	glutSolidSphere(0.1f, 5, 5);
	glTranslatef(0.0f, -1.5f, 0.0f);
	glColor3f(0.75f, 0.75f, 0.75f);
	glScalef(1.0f, 0.1f, 1.0f);
	glutSolidSphere(0.1f, 5, 5);
	glPopMatrix();

	// Draw some scale lines
	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINES);
	for (unsigned i = 0; i < 200; i += 10)
	{
		glVertex3f(-5.0f, 0.0f, i);
		glVertex3f(5.0f, 0.0f, i);
	}
	glEnd();

	// Render each particle in turn
	glColor3f(1,0,0);
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type != UNUSED)
		{
			shot->render();
		}
	}

	// Render the box
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(1,0,0);
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		box->calculateInternals();
        box->isOverlapping = false;
		box->render();
	}
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// Render the description
	glColor3f(0.0f, 0.0f, 0.0f);
	renderText(10.0f, 34.0f, "Click: Fire\n1-4: Select Ammo");

	// Render the name of the current shot type
	switch(currentShotType)
	{
	case PISTOL: renderText(10.0f, 10.0f, "Current Ammo: Pistol"); break;
	case ARTILLERY: renderText(10.0f, 10.0f, "Current Ammo: Artillery"); break;
	case FIREBALL: renderText(10.0f, 10.0f, "Current Ammo: Fireball"); break;
	case LASER: renderText(10.0f, 10.0f, "Current Ammo: Laser"); break;
	}
}

void BigBallisticDemo::generateContacts()
{
	// Create the ground plane data
	cyclone::CollisionPlane plane;
	plane.direction = cyclone::Vector3(0,1,0);
	plane.offset = 0;

	// Set up the collision data structure
	cData.reset(maxContacts);
	cData.friction = (cyclone::real)0.9;
	cData.restitution = (cyclone::real)0.1;
	cData.tolerance = (cyclone::real)0.1;

	// Check ground plane collisions
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		if (!cData.hasMoreContacts()) return;
		cyclone::CollisionDetector::boxAndHalfSpace(*box, plane, &cData);


		// Check for collisions with each shot
		for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
		{
			if (shot->type != UNUSED)
			{
				if (!cData.hasMoreContacts()) return;

				// When we get a collision, remove the shot
				if (cyclone::CollisionDetector::boxAndSphere(*box, *shot, &cData))
				{
					shot->type = UNUSED;
				}
			}
		}

		// Check for collisions with each other box
        for (Box *other = box+1; other < boxData+boxes; other++)
        {
            if (!cData.hasMoreContacts()) return;
            cyclone::CollisionDetector::boxAndBox(*box, *other, &cData);

            if (cyclone::IntersectionTests::boxAndBox(*box, *other))
            {
                box->isOverlapping = other->isOverlapping = true;
            }
        }
	}

	// NB We aren't checking box-box collisions.
}

void BigBallisticDemo::mouse(int button, int state, int x, int y)
{
	// Fire the current weapon.
	if (state == GLUT_DOWN) fire();
}

void BigBallisticDemo::key(unsigned char key)
{
	switch(key)
	{
	case '1': currentShotType = PISTOL; break;
	case '2': currentShotType = ARTILLERY; break;
	case '3': currentShotType = FIREBALL; break;
	case '4': currentShotType = LASER; break;

	case 'r': reset(); break;

	case 'a': x1+=1.0; break;
	case 's': y1+=1.0; break;
	case 'd': z1+=1.0; break;
	case 'z': x1-=1.0; break;
	case 'x': y1-=1.0; break;
	case 'c': z1-=1.0; break;

	case 'h': x2+=1.0; break;
	case 'j': y2+=1.0; break;
	case 'k': z2+=1.0; break;
	case 'b': x2-=1.0; break;
	case 'n': y2-=1.0; break;
	case 'm': z2-=1.0; break;
	}

	std::cout << "x1:" << x1 << " y1:" << y1 << " z1:" << z1 << std::endl;
	std::cout << "x2:" << x2 << " y2:" << y2 << " z2:" << z2 << std::endl;

}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
	return new BigBallisticDemo();
}