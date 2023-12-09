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

#include "ev3nodepublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>
#include "ev3data.h"
#include "ev3dev.h"

using namespace eprosima::fastdds::dds;


Ev3NodePublisher::Ev3NodePublisher() : myType(new Ev3SensorEventPubSubType()) /* object managed by TypeSupport class */
{
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    myParticipant = DomainParticipantFactory::get_instance()->create_participant(myDomain, pqos);
    if (myParticipant == nullptr) {
        throw std::runtime_error("Failed creating domain participant");
    }  

    myPublisher = myParticipant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (myPublisher == nullptr) {
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 node publisher");
    }

    mySensorEventTopic = myParticipant->create_topic("ev3_sensor_event_topic", "Ev3SensorEvent", TOPIC_QOS_DEFAULT);
    if (mySensorEventTopic == nullptr)  {
        myParticipant->delete_publisher(myPublisher);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event topic");
    }

    myWriter = myPublisher->create_datawriter(mySensorEventTopic, DATAWRITER_QOS_DEFAULT, &myListener);
    if (myWriter == nullptr)  {
        myParticipant->delete_topic(mySensorEventTopic);
        myParticipant->delete_publisher(myPublisher);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event writer");
    }
    myType.register_type(myParticipant);

    struct ButtonPair
    {
        ev3dev::button& b;
        enum Ev3Button bKind;
    };
    ButtonPair buttons[] = {
        { ev3dev::button::back, Back },
        { ev3dev::button::left, Left },
        { ev3dev::button::up, Up },
        { ev3dev::button::right, Right },
        { ev3dev::button::down, Down },
        { ev3dev::button::enter, Enter }
    };

    for (auto& bpair: buttons) {
        enum Ev3Button kind = bpair.bKind;
        bpair.b.onclick = [this, kind](bool pressed) {
            Ev3ButtonEvent event;
            event.button(kind);
            event.pressed(pressed);
            myWriter->write(&event);
        };
    }
}


Ev3NodePublisher::~Ev3NodePublisher()
{
    ev3dev::button* buttons[] = {   &ev3dev::button::back,  &ev3dev::button::left, 
                                    &ev3dev::button::up,    &ev3dev::button::right, 
                                    &ev3dev::button::down,  &ev3dev::button::enter  };

    //Disconnect button objects.
    for (auto& b : buttons) {
        b->onclick = [](bool){};
    }
    assert(myPublisher != nullptr);
    myPublisher->delete_datawriter(myWriter);
    assert(myParticipant != nullptr);
    myParticipant->delete_publisher(myPublisher);
    myParticipant->delete_topic(mySensorEventTopic);
    DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
}

void Ev3NodePublisher::process_sensors()
{
    ev3dev::button::process_all();
}
