/*
 * Implementation file for the particle force generators.
 *
 * Part of the Cyclone physics system.
 *
 * Copyright (c) Icosagon 2003. All Rights Reserved.
 *
 * This software is distributed under licence. Use of this software
 * implies agreement with all terms and conditions of the accompanying
 * software licence.
 */

#include <cyclone/pfgen.h>

using namespace cyclone;

// 5.1
ParticleUplift::ParticleUplift(const Vector3 &upforce, const Vector3 &origin, real rad)
: upforce(upforce), origin(origin), rad(rad)
{
}

void ParticleUplift::updateForce(Particle *particle, real duration) {
    // check if particle is within uplift distance
    if(particle->getPosition().x >= origin.x && particle->getPosition().x <= origin.x + rad) {
        if(particle->getPosition().z >= origin.z && particle->getPosition().z <= origin.z + rad) {
            // add force if within uplift distance
            particle->addForce(upforce);
        }
    }
}

// 5.2
ParticleAirbrake::ParticleAirbrake(real k1, real k2, bool brake)
: k1(k1), k2(k2), brake(brake)
{
}

void ParticleAirbrake::updateForce(Particle* particle, real duration)
{
    Vector3 force;

    // check if particle is braking
    if(brake) {
        particle->getVelocity(&force);

        // Calculate the total drag coefficient
        real dragCoeff = force.magnitude();
        dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;

        // Calculate the final force and apply it
        force.normalise();
        force *= -dragCoeff;
        particle->addForce(force);
    }
    
}

// 5.3 & 5.4
ParticleGravityAttract::ParticleGravityAttract(const Vector3 &point, real gravity, real m2)
: gravity(gravity), point(point), m2(m2)
{
}

void ParticleGravityAttract::updateForce(Particle* particle, real duration)
{
    // Check that we do not have infinite mass
    if (!particle->hasFiniteMass()) return;

    // get position of particle
    Vector3 pos;
    particle->getPosition(&pos);

    // set result to be other object and get vector result of p-x
    Vector3 result = point;
    result -= pos;

    // get square magnitude of result vector and then normalise
    real r2 = result.squareMagnitude();
    result.normalise();

    // use law of gravitation to get resulting force
    Vector3 attractForce = result;
    attractForce *= ((gravity * particle->getMass() * m2)/r2);

    // Apply the force to the particle
    particle->addForce(attractForce);
}

// 6.1
ParticleLimitedSpring::ParticleLimitedSpring(Particle *other, real sc, real rl, real maxDist)
: other(other), springConstant(sc), restLength(rl), maxDist(maxDist)
{
}

void ParticleLimitedSpring::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= other->getPosition();

    if(force.magnitude() > maxDist) {
    	springConstant *= 0.5f;
    }

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    magnitude = real_abs(magnitude - restLength);
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}
/*
// 6.2
ParticleAirBuoyancy::ParticleAirBuoyancy(const Vector3& gravity, real volume, real airDensity)
:
gravity(gravity), volume(volume), airDensity(airDensity)
{
}

void ParticleAirBuoyancy::updateForce(Particle* particle, real duration)
{
    // Apply the mass-scaled force to the particle
    Vector3 gravForce = gravity * particle->getMass();

    // TODO change this for max altitude (when bouyant force = gravitational force)
    if (depth >= waterHeight + maxDepth) return;
    Vector3 force(0,0,0);

    // Check if we're at maximum altitude
    if (depth <= waterHeight - maxDepth)
    {
        force.y = airDensity * volume;
        particle->addForce(force);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity * volume *
        (depth - maxDepth - waterHeight) / 2 * maxDepth;
    particle->addForce(force);
}
*/

// 6.2
ParticleAirBuoyancy::ParticleAirBuoyancy(real maxAltitude,
                                 real volume,
                                 real airDensity)
:
maxAltitude(maxAltitude), volume(volume), airDensity(airDensity)
{
}

void ParticleAirBuoyancy::updateForce(Particle* particle, real duration)
{
    // Calculate the altitude
    real altitude = particle->getPosition().y;

    Vector3 force;

    // Check if we're at maximum altitude
    if (altitude <= maxAltitude)
    {
        force.y = airDensity * volume * ((maxAltitude - altitude)*0.01f);
    }
    else {
        force.y = 0;
    }

    particle->addForce(force);
}


void ParticleForceRegistry::updateForces(real duration)
{
    Registry::iterator i = registrations.begin();
    for (; i != registrations.end(); i++)
    {
        i->fg->updateForce(i->particle, duration);
    }
}

void ParticleForceRegistry::add(Particle* particle, ParticleForceGenerator *fg)
{
    ParticleForceRegistry::ParticleForceRegistration registration;
    registration.particle = particle;
    registration.fg = fg;
    registrations.push_back(registration);
}

ParticleGravity::ParticleGravity(const Vector3& gravity)
: gravity(gravity)
{
}

void ParticleGravity::updateForce(Particle* particle, real duration)
{
    // Check that we do not have infinite mass
    if (!particle->hasFiniteMass()) return;

    // Apply the mass-scaled force to the particle
    particle->addForce(gravity * particle->getMass());
}

ParticleDrag::ParticleDrag(real k1, real k2)
: k1(k1), k2(k2)
{
}

void ParticleDrag::updateForce(Particle* particle, real duration)
{
    Vector3 force;
    particle->getVelocity(&force);

    // Calculate the total drag coefficient
    real dragCoeff = force.magnitude();
    dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;

    // Calculate the final force and apply it
    force.normalise();
    force *= -dragCoeff;
    particle->addForce(force);
}

ParticleSpring::ParticleSpring(Particle *other, real sc, real rl)
: other(other), springConstant(sc), restLength(rl)
{
}

void ParticleSpring::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= other->getPosition();

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    magnitude = real_abs(magnitude - restLength);
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

ParticleBuoyancy::ParticleBuoyancy(real maxDepth,
                                 real volume,
                                 real waterHeight,
                                 real liquidDensity)
:
maxDepth(maxDepth), volume(volume),
waterHeight(waterHeight), liquidDensity(liquidDensity)
{
}

void ParticleBuoyancy::updateForce(Particle* particle, real duration)
{
    // Calculate the submersion depth
    real depth = particle->getPosition().y;

    // Check if we're out of the water
    if (depth >= waterHeight + maxDepth) return;
    Vector3 force(0,0,0);

    // Check if we're at maximum depth
    if (depth <= waterHeight - maxDepth)
    {
        force.y = liquidDensity * volume;
        particle->addForce(force);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity * volume *
        (depth - maxDepth - waterHeight) / 2 * maxDepth;
    particle->addForce(force);
}

ParticleBungee::ParticleBungee(Particle *other, real sc, real rl)
: other(other), springConstant(sc), restLength(rl)
{
}

void ParticleBungee::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= other->getPosition();

    // Check if the bungee is compressed
    real magnitude = force.magnitude();
    if (magnitude <= restLength) return;

    // Calculate the magnitude of the force
    magnitude = springConstant * (restLength - magnitude);

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

ParticleFakeSpring::ParticleFakeSpring(Vector3 *anchor, real sc, real d)
: anchor(anchor), springConstant(sc), damping(d)
{
}

void ParticleFakeSpring::updateForce(Particle* particle, real duration)
{
    // Check that we do not have infinite mass
    if (!particle->hasFiniteMass()) return;

    // Calculate the relative position of the particle to the anchor
    Vector3 position;
    particle->getPosition(&position);
    position -= *anchor;

    // Calculate the constants and check they are in bounds.
    real gamma = 0.5f * real_sqrt(4 * springConstant - damping*damping);
    if (gamma == 0.0f) return;
    Vector3 c = position * (damping / (2.0f * gamma)) +
        particle->getVelocity() * (1.0f / gamma);

    // Calculate the target position
    Vector3 target = position * real_cos(gamma * duration) +
        c * real_sin(gamma * duration);
    target *= real_exp(-0.5f * duration * damping);

    // Calculate the resulting acceleration and therefore the force
    Vector3 accel = (target - position) * ((real)1.0 / (duration*duration)) -
        particle->getVelocity() * ((real)1.0/duration);
    particle->addForce(accel * particle->getMass());
}

ParticleAnchoredSpring::ParticleAnchoredSpring()
{
}

ParticleAnchoredSpring::ParticleAnchoredSpring(Vector3 *anchor,
                                               real sc, real rl)
: anchor(anchor), springConstant(sc), restLength(rl)
{
}

void ParticleAnchoredSpring::init(Vector3 *anchor, real springConstant,
                                  real restLength)
{
    ParticleAnchoredSpring::anchor = anchor;
    ParticleAnchoredSpring::springConstant = springConstant;
    ParticleAnchoredSpring::restLength = restLength;
}

void ParticleAnchoredBungee::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= *anchor;

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    if (magnitude < restLength) return;

    magnitude = magnitude - restLength;
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

void ParticleAnchoredSpring::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= *anchor;

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    magnitude = (restLength - magnitude) * springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= magnitude;
    particle->addForce(force);
}
