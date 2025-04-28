#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Atom
{
public:
    std::string type;         // Atom type
    float mass;               // Mass of the atom
    glm::vec3 position;       // 3D position
    glm::vec3 velocity;       // Current velocity
    glm::vec3 color;          // Color based on type
    float radius;             // Radius (for rendering size)
    bool fixed = false;       // If true, atom doesn't move
    int lonePairs = 0;
	int formalCharge = 0; // Formal charge
    glm::vec3 polarityDir{ 0.f }; // Gives the direction in 3D space


    std::vector<Atom *> bondedAtoms; // List of atoms this atom is bonded to

    Atom( const std::string &type, const glm::vec3 &pos, float mass );

    void update( float dt );
    bool isAlive() const;
};
