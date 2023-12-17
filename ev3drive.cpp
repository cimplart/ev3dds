#include "ev3drive.h"
#include "ev3data.h"
#include <map>

static std::map<enum Ev3MoveType, std::string> moveTypeStr = {
    { Ev3MoveType::Forward, "Forward" },
    { Ev3MoveType::ForwardLeft, "ForwardLeft" }, 
    { Ev3MoveType::ForwardRight, "ForwardRight" },
    { Ev3MoveType::SpinLeft, "SpinLeft" },  
    { Ev3MoveType::SpinRight, "SpinRight" },
    { Ev3MoveType::Backward, "Backward" }
};


Ev3Drive::Ev3Drive()
{

}

Ev3Drive::~Ev3Drive()
{

}

void Ev3Drive::on_move_command(const Ev3MoveCommand& cmd)
{
    std::cout << "Move command: " << (cmd.make() ? "make " : "stop ") << moveTypeStr[cmd.move()] << " move\n";
}