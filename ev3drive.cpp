// MIT License

// Copyright (c) 2023 Artur Wisz

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
