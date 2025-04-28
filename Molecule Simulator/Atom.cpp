#include "Atom.h"

const glm::vec3 gravity( 0.0f, 0.0f, 0.0f );
const float boxSize = 5.0f;

Atom::Atom( const std::string &type, const glm::vec3 &pos, float mass )
    : type( type ), mass( mass ), position( pos ), velocity( glm::vec3( 0.0f ) ), radius( 0.2f ) {

    if (type == "O")  color = glm::vec3( 1.00f, 0.00f, 0.00f ); // red      – Oxygen
    else if (type == "H")  color = glm::vec3( 1.00f, 1.00f, 1.00f ); // white    – Hydrogen
    else if (type == "C")  color = glm::vec3( 0.20f, 0.20f, 0.20f ); // dark gray– Carbon
    else if (type == "N")  color = glm::vec3( 0.00f, 0.00f, 1.00f ); // blue     – Nitrogen

    // common hetero atoms
    else if (type == "S")  color = glm::vec3( 1.00f, 1.00f, 0.18f ); // yellow   – Sulfur
    else if (type == "P")  color = glm::vec3( 1.00f, 0.50f, 0.00f ); // orange   – Phosphorus

    // halogens
    else if (type == "F")  color = glm::vec3( 0.00f, 0.90f, 0.00f ); // green    – Fluorine
    else if (type == "Cl") color = glm::vec3( 0.00f, 0.80f, 0.00f ); // green    – Chlorine
    else if (type == "Br") color = glm::vec3( 0.65f, 0.16f, 0.16f ); // dark red – Bromine
    else if (type == "I")  color = glm::vec3( 0.58f, 0.00f, 0.83f ); // violet   – Iodine

    // noble gases
    else if (type == "He") color = glm::vec3( 0.85f, 1.00f, 1.00f ); // cyan     – Helium
    else if (type == "Ne") color = glm::vec3( 0.70f, 0.89f, 0.96f ); // pale cyan– Neon
    else if (type == "Ar") color = glm::vec3( 0.50f, 0.82f, 0.89f ); // cyan     – Argon

    // alkali & alkaline-earth metals
    else if (type == "Li") color = glm::vec3( 0.80f, 0.50f, 1.00f ); // purple   – Lithium
    else if (type == "Na") color = glm::vec3( 0.00f, 0.00f, 0.80f ); // royal blue– Sodium
    else if (type == "K")  color = glm::vec3( 0.56f, 0.25f, 0.83f ); // violet   – Potassium
    else if (type == "Mg") color = glm::vec3( 0.13f, 0.54f, 0.13f ); // dark green– Magnesium
    else if (type == "Ca") color = glm::vec3( 0.24f, 1.00f, 0.00f ); // green    – Calcium

    // selected transition metals
    else if (type == "Fe") color = glm::vec3( 0.88f, 0.40f, 0.20f ); // orange   – Iron
    else if (type == "Cu") color = glm::vec3( 0.78f, 0.50f, 0.20f ); // brown-gold– Copper
    else if (type == "Zn") color = glm::vec3( 0.49f, 0.50f, 0.69f ); // slate    – Zinc

    // miscellaneous p-block
    else if (type == "B")  color = glm::vec3( 1.00f, 0.67f, 0.47f ); // salmon   – Boron
    else if (type == "Si") color = glm::vec3( 0.94f, 0.78f, 0.62f ); // beige    – Silicon
    else if (type == "Al") color = glm::vec3( 0.75f, 0.65f, 0.65f ); // light gray– Aluminum

    // default fallback
    else                   color = glm::vec3( 0.50f, 0.50f, 0.50f ); // light gray

}

void Atom::update( float dt ) {
    if (fixed) return;

    velocity += gravity * dt;
    position += velocity * dt;

    // Damping (air resistance) to slow down
    velocity *= 0.98f; // 2% speed loss every frame

    // Bounce inside cube (wall collision)
    if (position.x < -boxSize)
    {
        position.x = -boxSize; velocity.x *= -0.6f;
    }
    if (position.x > boxSize)
    {
        position.x = boxSize; velocity.x *= -0.6f;
    }
    if (position.y < -boxSize)
    {
        position.y = -boxSize; velocity.y *= -0.6f;
    }
    if (position.y > boxSize)
    {
        position.y = boxSize; velocity.y *= -0.6f;
    }
    if (position.z < -boxSize)
    {
        position.z = -boxSize; velocity.z *= -0.6f;
    }
    if (position.z > boxSize)
    {
        position.z = boxSize; velocity.z *= -0.6f;
    }
}

bool Atom::isAlive() const {
    return true; 
}
