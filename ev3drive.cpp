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


Ev3Drive::Ev3Drive() :
  myLeftMotor(ev3dev::OUTPUT_C),
  myRightMotor(ev3dev::OUTPUT_B)
{
    myLeftMotor.set_stop_action("brake");
    myRightMotor.set_stop_action("brake");
}

Ev3Drive::~Ev3Drive()
{

}

void Ev3Drive::on_move_command(const Ev3MoveCommand& cmd)
{
    std::cout << "Move command: " << (cmd.make() ? "make " : "stop ") << moveTypeStr[cmd.move()] << " move\n";

    if (cmd.make()) {
        switch(cmd.move()) {
            case Ev3MoveType::Forward:
                drive(SPEED, SPEED);
                break;
            case Ev3MoveType::ForwardLeft:
                drive(SPEED/2, SPEED);
                break;
            case Ev3MoveType::ForwardRight:
                drive(SPEED, SPEED/2);
                break;
            case Ev3MoveType::SpinLeft:
                drive(0, SPEED);
                break;
            case Ev3MoveType::SpinRight:
                drive(SPEED, 0);
                break;
            case Ev3MoveType::Backward:
                drive(-SPEED, -SPEED);
                break;
        }
    } else {
        stop();
    }
}

void Ev3Drive::drive(int leftSpeed, int rightSpeed)
{
    myLeftMotor.set_speed_sp(-leftSpeed);
    myRightMotor.set_speed_sp(-rightSpeed);

    myLeftMotor.run_forever();
    myRightMotor.run_forever();
}

void Ev3Drive::stop()
{
    myLeftMotor .stop();
    myRightMotor.stop();
}
