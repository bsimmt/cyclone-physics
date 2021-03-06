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
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <iomanip>
#include <sstream>

using namespace std;

enum ShotType
{
	UNUSED = 0,
	P1ARTILLERY,
	P1BOMB,
	P2ARTILLERY,
	P2BOMB
};

class AmmoRound : public cyclone::CollisionSphere
{
public:
	ShotType type;
	unsigned startTime;
	float grav;
	float wind;
	float aimX;
	float aimY;
	float p1pos;
	float p2pos;

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

	void setGravity(float grav) {
		AmmoRound::grav = grav;
	}

	void setAim(float aimX, float aimY) {
		AmmoRound::aimX = aimX;
		AmmoRound::aimY = aimY;
	}

	void setPos(float p1pos, float p2pos) {
		AmmoRound::p1pos = p1pos;
		AmmoRound::p2pos = p2pos;
	}

	/** Sets the box to a specific location. */
	void setState(ShotType shotType)
	{
		type = shotType;
		// randomize wind per shot
		wind = (float)(-20 + rand() % 40);

		// Set the properties of the particle
		switch(type)
		{
		case P1ARTILLERY:
			body->setMass(750.0f); // 200.0kg
			body->setVelocity(0.0f, aimY, aimX); // 50m/s
			body->setAcceleration(0.0f, grav, wind);
			body->setDamping(0.99f, 0.8f);
			body->setPosition(0.0f, 1.5f, p1pos);
			radius = 2.0f;
			break;

		case P1BOMB:
			body->setMass(10000.0f); // 200.0kg
			body->setVelocity(0.0f, 100.0f, 30.0f); // 50m/s
			body->setAcceleration(0.0f, grav, wind/10);
			body->setDamping(0.50f, 0.8f);
			body->setPosition(0.0f, 1.5f, p1pos);
			radius = 5.0f;
			break;

		case P2ARTILLERY:
			body->setMass(750.0f); // 200.0kg
			body->setVelocity(0.0f, aimY, -aimX); // 50m/s
			body->setAcceleration(0.0f, grav, wind);
			body->setDamping(0.99f, 0.8f);
			body->setPosition(0.0f, 1.5f, p2pos);
			radius = 2.0f;
			break;

		case P2BOMB:
			body->setMass(10000.0f); // 200.0kg
			body->setVelocity(0.0f, 100.0f, -30.0f); // 50m/s
			body->setAcceleration(0.0f, grav, wind/10);
			body->setDamping(0.50f, 0.8f);
			body->setPosition(0.0f, 1.5f, p2pos);
			radius = 5.0f;
			break;
		}

		body->setCanSleep(false);
		body->setAwake();

		cyclone::Matrix3 tensor;
		cyclone::real coeff = 0.4f*body->getMass()*radius*radius;
		tensor.setInertiaTensorCoeffs(coeff,coeff,coeff);
		body->setInertiaTensor(tensor);

		// Set the data common to all particle types
		//body->setPosition(0.0f, 1.5f, 0.0f);
		startTime = TimingData::get().lastFrameTimestamp;

		// Clear the force accumulators
		body->calculateDerivedData();
		calculateInternals();
	}

	float getWind() {
		return wind;
	}

};

class Box : public cyclone::CollisionBox
{
public:
	bool isOverlapping;
	float randMass;

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

        glColor3f(0.0f,1.0f,0.0f);

        if(randMass > 75) {
        	glColor3f(1.0f,0.0f,0.0f);
        }
        else if(randMass > 50) {
        	glColor3f(0.5f,0.0f,0.0f);
        }
        else if(randMass > 25) {
        	glColor3f(0.5f,1.0f,0.7f);
        }
        else if(body->getMass()==1000) {
        	glColor3f(1.0f,-1.0f,-1.0f);
        }

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

		randMass = (float)(rand() % 100);\
		for(int i=0; i<1; i++) {
			if(randMass > 50) {
				randMass = (float)(rand() % 100);
			}
		}
		std::cout << "randMass: " << to_string(randMass) << std::endl;
		cyclone::real mass = halfSize.x+randMass * halfSize.y+randMass * halfSize.z+randMass * 1.0f;
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

	/** Sets the box to a specific location. */
	void setFixedState(cyclone::real y, cyclone::real z)
	{
		body->setPosition(0, y, z);
		body->setOrientation(1,0,0,0);
		body->setVelocity(0,0,0);
		body->setRotation(cyclone::Vector3(0,0,0));
		halfSize = cyclone::Vector3(5,5,5);

		cyclone::real mass = halfSize.x * halfSize.y * halfSize.z * 1.0f;
		body->setMass(1000);

		cyclone::Matrix3 tensor;
		tensor.setBlockInertiaTensor(halfSize, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(1.0f);
		body->setAngularDamping(1.0f);
		body->clearAccumulators();
		body->setAcceleration(0,-10.0f,0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();
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
	const static unsigned boxes = 22;

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

	virtual void renderString(string text1, string text2, float stats, float x, float y);

	virtual void renderString(string text1, float x, float y);

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

	float grav;
	float wind;
	float aimX;
	float aimY;

	float p1hp;
	float p2hp;

	float p1pos;
	float p2pos;

	string shotType;

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
currentShotType(P1ARTILLERY)
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
	aimX = 40.0f;
	aimY = 60.0f;

	p1hp = 1000.0f;
	p2hp = 1000.0f;

	p1pos = 0.0f;
	p2pos = 120.0f;

	shotType = "P1 Artillery";
	// randomize gravity on round reset
	grav = (float)(-60 + rand() % 25);

	// Make all shots unused
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		shot->setGravity(grav);
		shot->setAim(aimX, aimY);
		shot->type = UNUSED;
	}

	// Initialise the box
	cyclone::real initial = 40.0f;
	cyclone::real z = initial;
	cyclone::real y = 5.0f;
	int i = 0;
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		box->setState(y, z);
		z += 11.0f;
		i++;
		if(i%5 == 0) {
			z = initial;
			y += 11.0f;
		}
		if(i==boxes-1) {
			box->setFixedState(5.0f, p1pos);
		}
		else if(i==boxes) {
			box->setFixedState(5.0f, p2pos);
		}
	}
}

const char* BigBallisticDemo::getTitle()
{
	return "Tank Game";
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
	shot->setAim(aimX, aimY);
	shot->setState(currentShotType);
	wind = shot->getWind();
}

void BigBallisticDemo::updateObjects(cyclone::real duration)
{
	if(p1pos > 25) {
		p1pos--;
	}
	else if(p1pos < -25) {
		p1pos++;
	}

	if(p2pos < 80) {
		p2pos++;
	}
	else if(p2pos > 140) {
		p2pos--;
	}

	// Update the physics of each particle in turn
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		shot->setPos(p1pos, p2pos);
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

	int boxIndex = 0;
	// Update the boxes
	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		if(boxIndex == 20) {
			box->setFixedState(5.0f, p1pos);
		}
		else if(boxIndex == 21) {
			box->setFixedState(5.0f, p2pos);
		}
		else {
			// Run the physics
			box->body->integrate(duration);
			box->calculateInternals();
			box->isOverlapping = false;
		}
		boxIndex++;
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
	glColor3f(0,0,0);
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

	BigBallisticDemo::renderString("Gravity: ", " m/s^2", grav/5, 10.0f, 30.0f);
	BigBallisticDemo::renderString("Wind: ", " m/s", wind/2, 10.0f, 10.0f);
	BigBallisticDemo::renderString("P1 HP: ", "", p1hp, 10.0f, 300.0f);
	BigBallisticDemo::renderString("P2 HP: ", "", p2hp, 10.0f, 310.0f);
	BigBallisticDemo::renderString("Aiming: ", "", aimX, 10.0f, 50.0f);
	BigBallisticDemo::renderString(shotType, 10.0f, 70.0f);
}

void BigBallisticDemo::renderString(string text1, string text2, float stats, float x, float y) {
	stringstream stream;
	stream << fixed << setprecision(2) << stats;
	string s = stream.str();
	string rText = text1 + s + text2;
	int n = rText.length(); 
    char charArr[n+1];
    strcpy(charArr, rText.c_str());
    renderText(x, y, charArr);
}

void BigBallisticDemo::renderString(string text1, float x, float y) {
	string rText = text1;
	int n = rText.length(); 
    char charArr[n+1];
    strcpy(charArr, rText.c_str());
    renderText(x, y, charArr);
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

	}

	int boxIndex = 0;

	for (Box *box = boxData; box < boxData+boxes; box++)
	{
		// Check for collisions with each shot
		for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
		{
			if (shot->type != UNUSED)
			{

				if(boxIndex == 20) {
					if (shot->type == 3 || shot->type == 4) {
						if (!cData.hasMoreContacts()) return;

						// When we get a collision, remove the shot
						if (cyclone::CollisionDetector::boxAndSphere(*box, *shot, &cData))
						{
							shot->type = UNUSED;
							float damage = shot->body->getVelocity().z * (shot->body->getMass()/750);
							p1hp -= -damage;
							if(-damage > 50) {
								p1pos-=2;
							}
						}
						
					}
				}
				else if(boxIndex == 21) {
					if (shot->type == 1 || shot->type == 2) {
						if (!cData.hasMoreContacts()) return;

						// When we get a collision, remove the shot
						if (cyclone::CollisionDetector::boxAndSphere(*box, *shot, &cData))
						{
							shot->type = UNUSED;
							float damage = shot->body->getVelocity().z * (shot->body->getMass()/750);
							p2hp -= damage;
							if(damage > 50) {
								p2pos+=2;
							}
						}
					}
				}
				else {
					if (!cData.hasMoreContacts()) return;

					// When we get a collision, remove the shot
					if (cyclone::CollisionDetector::boxAndSphere(*box, *shot, &cData))
					{
						shot->type = UNUSED;
					}
				}
				if(p1hp < 0 || p2hp < 0) {
					reset();
				}
			}
		}

		// Check for collisions with each other box
        for (Box *other = box+1; other < boxData+boxes-2; other++)
        {
            if (!cData.hasMoreContacts()) return;
            cyclone::CollisionDetector::boxAndBox(*box, *other, &cData);

            if (cyclone::IntersectionTests::boxAndBox(*box, *other))
            {
                box->isOverlapping = other->isOverlapping = true;
            }
        }
        boxIndex++;
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
	case '1': currentShotType = P1ARTILLERY; shotType = "P1 Artillery"; break;
	case '2': currentShotType = P1BOMB; shotType = "P1 Bomb"; break;
	case '3': currentShotType = P2ARTILLERY; shotType = "P2 Artillery"; break;
	case '4': currentShotType = P2BOMB; shotType = "P2 Bomb"; break;

	case 'r': reset(); break;

	case 'w': aimX-=10.0f; aimY+=10.0f; break;
	case 's': aimX+=10.0f; aimY-=10.0f; break;

	case 'a': p1pos+=-1.0f; break;
	case 'd': p1pos+=1.0f; break;
	case 'j': p2pos+=-1.0f; break;
	case 'l': p2pos+=1.0f; break;
	}
}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
	return new BigBallisticDemo();
}