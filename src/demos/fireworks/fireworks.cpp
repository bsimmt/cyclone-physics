/*
 * The fireworks demo.
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
#include <sstream>
#include <string>

static cyclone::Random crandom;

/**
 * Fireworks are particles, with additional data for rendering and
 * evolution.
 */
class Firework : public cyclone::Particle
{
public:
    /** Fireworks have an integer type, used for firework rules. */
    unsigned type;

    /**
     * The age of a firework determines when it detonates. Age gradually
     * decreases, when it passes zero the firework delivers its payload.
     * Think of age as fuse-left.
     */
    cyclone::real age;

    /**
     * Updates the firework by the given duration of time. Returns true
     * if the firework has reached the end of its life and needs to be
     * removed.
     */
    bool update(cyclone::real duration)
    {
        // Update our physical state
        integrate(duration);

        // We work backwards from our age to zero.
        age -= duration;
        return (position.y < 0); /** returns boolean if ready to execute
        										* payload based on age, or if it hits
        										* the bottom of the field
        										*/
    }
};

/**
 * Firework rules control the length of a firework's fuse and the
 * particles it should evolve into.
 */
struct FireworkRule
{
    /** The type of firework that is managed by this rule. */
    unsigned type;

    /** The minimum length of the fuse. */
    cyclone::real minAge;

    /** The maximum legnth of the fuse. */
    cyclone::real maxAge;

    /** The minimum relative velocity of this firework. */
    cyclone::Vector3 minVelocity;

    /** The maximum relative velocity of this firework. */
    cyclone::Vector3 maxVelocity;

    /** The damping of this firework type. */
    cyclone::real damping;

    /**
     * The payload is the new firework type to create when this
     * firework's fuse is over.
     */
    struct Payload
    {
        /** The type of the new particle to create. */
        unsigned type;

        /** The number of particles in this payload. */
        unsigned count;

        /** Sets the payload properties in one go. */
        void set(unsigned type, unsigned count)
        {
            Payload::type = type;
            Payload::count = count;
        }
    };

    /** The number of payloads for this firework type. */
    unsigned payloadCount;

    /** The set of payloads. */
    Payload *payloads;

    FireworkRule()
    :
    payloadCount(0),
    payloads(NULL)
    {
    }

    void init(unsigned payloadCount)
    {
        FireworkRule::payloadCount = payloadCount;
        payloads = new Payload[payloadCount];
    }

    ~FireworkRule()
    {
        if (payloads != NULL) delete[] payloads;
    }

    /**
     * Set all the rule parameters in one go.
     */
    void setParameters(unsigned type, cyclone::real minAge, cyclone::real maxAge,
        const cyclone::Vector3 &minVelocity, const cyclone::Vector3 &maxVelocity,
        cyclone::real damping)
    {
        FireworkRule::type = type;
        FireworkRule::minAge = minAge;
        FireworkRule::maxAge = maxAge;
        FireworkRule::minVelocity = minVelocity;
        FireworkRule::maxVelocity = maxVelocity;
        FireworkRule::damping = damping;
    }

    /**
     * Creates a new firework of this type and writes it into the given
     * instance. The optional parent firework is used to base position
     * and velocity on.
     */
    void create(Firework *firework, const Firework *parent = NULL) const
    {
        firework->type = type;
        firework->age = crandom.randomReal(minAge, maxAge);

        cyclone::Vector3 vel;
        if (parent) {
            // The position and velocity are based on the parent.
            firework->setPosition(parent->getPosition());
            vel += parent->getVelocity();
        }
        else
        {
            cyclone::Vector3 start;
            int x = (int)crandom.randomInt(3) - 1;
            //start.x = 5.0f * cyclone::real(x);
            start.x = 5.0f * cyclone::real(0);
            firework->setPosition(start);
        }

        vel += crandom.randomVector(minVelocity, maxVelocity);
        firework->setVelocity(vel);

        // We use a mass of one in all cases (no point having fireworks
        // with different masses, since they are only under the influence
        // of gravity).
        firework->setMass(1);

        firework->setDamping(damping);

        firework->setAcceleration(cyclone::Vector3::GRAVITY);

        firework->clearAccumulator();
    }
};

/**
 * The main demo class definition.
 */
class FireworksDemo : public Application
{
    /**
     * Holds the maximum number of fireworks that can be in use.
     */
    const static unsigned maxFireworks = 1024;
    
    /** Keep track of game score */
    int score = 0;
    
    /** Mouse pressed state */
    bool isPressed = false;
    
    /** Firework hit state */
    bool isHit = false;
    
    /** Mouse coordinates */
    float mousex = 0;
    float mousey = 0;

    /** Holds the firework data. */
    Firework fireworks[maxFireworks];

    /** Holds the index of the next firework slot to use. */
    unsigned nextFirework;

    /** And the number of rules. */
    const static unsigned ruleCount = 11;

    /** Holds the set of rules. */
    FireworkRule rules[ruleCount];

    /** Dispatches a firework from the origin. */
    void create(unsigned type, const Firework *parent=NULL);

    /** Dispatches the given number of fireworks from the given parent. */
    void create(unsigned type, unsigned number, const Firework *parent);

    /** Creates the rules. */
    void initFireworkRules();
public:
    /** Creates a new demo object. */
    FireworksDemo();
    ~FireworksDemo();

    /** Sets up the graphic rendering. */
    virtual void initGraphics();

    /** Returns the window title for the demo. */
    virtual const char* getTitle();

    /** Update the particle positions. */
    virtual void update();

    /** Display the particle positions. */
    virtual void display();

    /** Handle a keypress. */
    virtual void key(unsigned char key);
    
    /** Handle a mouse click. */
    virtual void mouse(int button, int state, int x, int y);
};

// Method definitions
FireworksDemo::FireworksDemo()
:
nextFirework(0)
{
    // Make all shots unused
    for (Firework *firework = fireworks;
         firework < fireworks+maxFireworks;
         firework++)
    {
        firework->type = 0;
    }

    // Create the firework types
    initFireworkRules();
}

FireworksDemo::~FireworksDemo()
{
}

void FireworksDemo::initFireworkRules()
{
    // Go through the firework types and create their rules.
    rules[0].init(2);
    rules[0].setParameters(
        1, // type
        0.5f, 1.4f, // age range
        cyclone::Vector3(-5, 25, -5), // min velocity
        cyclone::Vector3(5, 28, 5), // max velocity
        0.1 // damping
        );
    rules[0].payloads[0].set(3, 5);
    rules[0].payloads[1].set(5, 5);

    rules[1].init(1);
    rules[1].setParameters(
        2, // type
        0.5f, 1.0f, // age range
        cyclone::Vector3(-5, 10, -5), // min velocity
        cyclone::Vector3(5, 20, 5), // max velocity
        0.8 // damping
        );
    rules[1].payloads[0].set(4, 2);

    rules[2].init(0);
    rules[2].setParameters(
        3, // type
        0.5f, 1.5f, // age range
        cyclone::Vector3(-5, -5, -5), // min velocity
        cyclone::Vector3(5, 5, 5), // max velocity
        0.1 // damping
        );

    rules[3].init(0);
    rules[3].setParameters(
        4, // type
        0.25f, 0.5f, // age range
        cyclone::Vector3(-20, 5, -5), // min velocity
        cyclone::Vector3(20, 5, 5), // max velocity
        0.2 // damping
        );

    rules[4].init(1);
    rules[4].setParameters(
        5, // type
        0.5f, 1.0f, // age range
        cyclone::Vector3(-20, 2, -5), // min velocity
        cyclone::Vector3(20, 18, 5), // max velocity
        0.01 // damping
        );
    rules[4].payloads[0].set(3, 5);

    rules[5].init(0);
    rules[5].setParameters(
        6, // type
        3, 5, // age range
        cyclone::Vector3(-5, 5, -5), // min velocity
        cyclone::Vector3(5, 10, 5), // max velocity
        0.95 // damping
        );

    rules[6].init(3);
    rules[6].setParameters(
        7, // type
        3, 5, // age range
        cyclone::Vector3(-20, 25, 0), // min velocity
        cyclone::Vector3(20, 25, 0), // max velocity
        0.001 // damping
        );
	rules[6].payloads[0].set(10, 1);
	rules[6].payloads[1].set(7, 1);
	rules[6].payloads[2].set(8, 1);
    
    
    /** GAME RULES */
	rules[7].init(3);
    rules[7].setParameters(
        8, // type
        3, 5, // age range
        cyclone::Vector3(-10, 15, 0), // min velocity
        cyclone::Vector3(10, 15, 0), // max velocity
        0.001 // damping
        );
	rules[7].payloads[0].set(10, 1);
	rules[7].payloads[1].set(7, 1);
	rules[7].payloads[2].set(8, 1);


    rules[8].init(1);
    rules[8].setParameters(
        9, // type
        3, 5, // age range
        cyclone::Vector3(-25, 26, 0), // min velocity
        cyclone::Vector3(25, 26, 0), // max velocity
        0.01 // damping
        );
	rules[8].payloads[0].set(10, 1);
	
	rules[9].init(3);
    rules[9].setParameters(
        10, // type
        3, 5, // age range
        cyclone::Vector3(-15, 20, 0), // min velocity
        cyclone::Vector3(15, 20, 0), // max velocity
        0.001 // damping
        );
	rules[9].payloads[0].set(10, 1);
	rules[9].payloads[1].set(7, 1);
	rules[9].payloads[2].set(8, 1);
}

void FireworksDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* FireworksDemo::getTitle()
{
    return "Cyclone > Fireworks Demo";
}

void FireworksDemo::create(unsigned type, const Firework *parent)
{
    // Get the rule needed to create this firework
    FireworkRule *rule = rules + (type - 1);

    // Create the firework
    rule->create(fireworks+nextFirework, parent);

    // Increment the index for the next firework
    nextFirework = (nextFirework + 1) % maxFireworks;
}

void FireworksDemo::create(unsigned type, unsigned number, const Firework *parent)
{
    for (unsigned i = 0; i < number; i++)
    {
        create(type, parent);
    }
}

void FireworksDemo::update()
{
    // Find the duration of the last frame in seconds
    float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
    if (duration <= 0.0f) return;

    for (Firework *firework = fireworks;
         firework < fireworks+maxFireworks;
         firework++)
    {
        // Check if we need to process this firework.
        if (firework->type > 0)
        {
        	// Hit bottom then remove
        	if(firework->update(duration)) {
        		score -= 1;
        	 	// Delete the current firework
                firework->type = 0;
        	}
        
        	const static cyclone::real size = 0.1f;
			const cyclone::Vector3 &pos = firework->getPosition();
			if(isPressed) {
				if(mousex <= pos.x+(size*4) && mousex >= pos.x-(size*4)) {
					if(mousey <= pos.y+(size*4) && mousey >= pos.y-(size*4)) {
						// Find the appropriate rule
						FireworkRule *rule = rules + (firework->type-1);

						// Delete the current firework (this doesn't affect its
						// position and velocity for passing to the create function,
						// just whether or not it is processed for rendering or
						// physics.
						firework->type = 0;
							
						// Add the payload
						for (unsigned i = 0; i < rule->payloadCount; i++)
						{
							FireworkRule::Payload * payload = rule->payloads + i;
							create(payload->type, payload->count, firework);
							score += i;
						}
					}
				}	
			}
        }
    }

    Application::update();
}

void FireworksDemo::display()
{			
    const static cyclone::real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 10.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);

    // Render each firework in turn
    glBegin(GL_QUADS);
    for (Firework *firework = fireworks;
        firework < fireworks+maxFireworks;
        firework++)
    {
        // Check if we need to process this firework.
        if (firework->type > 0)
        {
            switch (firework->type)
            {
            case 1: glColor3f(1,0,0); break;
            case 2: glColor3f(1,0.5f,0); break;
            case 3: glColor3f(1,1,0); break;
            case 4: glColor3f(0,1,0); break;
            case 5: glColor3f(0,1,1); break;
            case 6: glColor3f(0.4f,0.4f,1); break;
            case 7: glColor3f(1,0,1); break;
            case 8: glColor3f(1,1,1); break;
            case 9: glColor3f(1,0.5f,0.5f); break;
            case 10: glColor3f(0.75f,0.5f,0.1f); break;
            };

            const cyclone::Vector3 &pos = firework->getPosition();
            glVertex3f(pos.x-size, pos.y-size, pos.z);
            glVertex3f(pos.x+size, pos.y-size, pos.z);
            glVertex3f(pos.x+size, pos.y+size, pos.z);
            glVertex3f(pos.x-size, pos.y+size, pos.z);
            
            
            /*
            glVertex3f(0-size, 0-size, pos.z);
            glVertex3f(0+size, 0-size, pos.z);
            glVertex3f(0+size, 0+size, pos.z);
            glVertex3f(0-size, 0+size, pos.z);
            
            glVertex3f(11.5-size, 5.5-size, pos.z);
            glVertex3f(11.5+size, 5.5-size, pos.z);
            glVertex3f(11.5+size, 5.5+size, pos.z);
            glVertex3f(11.5-size, 5.5+size, pos.z);
			
			glVertex3f(-11.5-size, -5.5-size, pos.z);
            glVertex3f(-11.5+size, -5.5-size, pos.z);
            glVertex3f(-11.5+size, -5.5+size, pos.z);
            glVertex3f(-11.5-size, -5.5+size, pos.z);
            */

			/*
            // Render the firework's reflection
            glVertex3f(pos.x-size, -pos.y-size, pos.z);
            glVertex3f(pos.x+size, -pos.y-size, pos.z);
            glVertex3f(pos.x+size, -pos.y+size, pos.z);
            glVertex3f(pos.x-size, -pos.y+size, pos.z);
            */
            
            /*
            if (isPressed) {
				std::cout << "firew_x: " << pos.x << std::endl;
				std::cout << "firew_y: " << pos.y << std::endl;
				std::cout << "mouse_x: " << mousex << std::endl;
				std::cout << "mouse_y: " << mousey << std::endl;
			}
			*/
            
           	
        }
    }
    
    /*
    std::cout << "mouse_x: " << mousex << std::endl;
    std::cout << "mouse_y: " << mousey << std::endl;
    */
    
    
    /*
    GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	
    std::cout<<"height:"<<GLUT_SCREEN_HEIGHT<<"\n";
    std::cout<<"height:"<<GLUT_SCREEN_HEIGHT<<"\n";*/

    glEnd();
    glColor3f(1,1,1);
    
    renderText(10.0f, 10.0f, ("score: " + std::to_string(score)).c_str());
    
    /*
    if (isPressed) {
		renderText(10.0f, 10.0f, "Mouse pressed%d", score);
	}
	else {
		renderText(10.0f, 10.0f, "Mouse Not Pressed");
	}*/
}

// check mouse position when clicked
void FireworksDemo::mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
    	isPressed = true;
    	mousex = ((x/(float)width) - 0.5)*23;
    	mousey = (-(y/(float)height) + 0.5)*11;
    }
    else {
    	isPressed = false;
    	mousex = 0;
    	mousey = 0;
    }
}

void FireworksDemo::key(unsigned char key)
{
    switch (key)
    {
    case '1': create(1, 1, NULL); break;
    case '2': create(2, 1, NULL); break;
    case '3': create(3, 1, NULL); break;
    case '4': create(4, 1, NULL); break;
    case '5': create(5, 1, NULL); break;
    case '6': create(6, 1, NULL); break;
    case '7': create(7, 1, NULL); break;
    case '8': create(8, 1, NULL); break;
    case '9': create(9, 1, NULL); break;
    case 'q': create(10, 1, NULL); break;
    case 'w': create(11, 1, NULL); break;
    }
}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new FireworksDemo();
}
