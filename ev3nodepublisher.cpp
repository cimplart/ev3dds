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

static std::map<enum Ev3Button, std::string> buttonStr = {
    { Back, "Back" },
    { Up, "Up" },
    { Down, "Down" },
    { Left, "Left" },
    { Right, "Right" },
    { Enter, "Enter" }
};

Ev3NodePublisher::Ev3NodePublisher() 
    : mySensorEventType(new Ev3SensorEventPubSubType()), /* object managed by TypeSupport class */
      myButtonEventType(new Ev3ButtonEventPubSubType())  /* object managed by TypeSupport class */
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    // This locator will open a socket to listen network messages
    // on UDPv4 port 7412 over address 192.168.50.244
    eprosima::fastrtps::rtps::Locator_t locator;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(locator, 192, 168, 50, 244);
    locator.port = 7412;
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);    

    // The initial peer address.
    eprosima::fastrtps::rtps::Locator_t initial_peer;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(initial_peer, 172, 17, 134, 54);
    pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer);

    pqos.name("Participant_pub");
    myParticipant = DomainParticipantFactory::get_instance()->create_participant(myDomain, pqos);
    if (myParticipant == nullptr) {
        throw std::runtime_error("Failed creating domain participant");
    }  

    mySensorEventType.register_type(myParticipant);
    myButtonEventType.register_type(myParticipant);

    myPublisher = myParticipant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (myPublisher == nullptr) {
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 node publisher");
    }

    mySensorEventTopic = myParticipant->create_topic("ev3_sensor_event_topic", myButtonEventType.get_type_name(), 
                                                     TOPIC_QOS_DEFAULT);
    if (mySensorEventTopic == nullptr)  {
        myParticipant->delete_publisher(myPublisher);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event topic");
    }

    DataWriterQos wqos = myPublisher->get_default_datawriter_qos();
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;  
    wqos.liveliness().lease_duration = 5;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;

    myWriter = myPublisher->create_datawriter(mySensorEventTopic, wqos, &myListener);
    if (myWriter == nullptr)  {
        myParticipant->delete_topic(mySensorEventTopic);
        myParticipant->delete_publisher(myPublisher);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event writer");
    }

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
            void* sample = myButtonEventType->createData();
            Ev3ButtonEvent *asEvent = static_cast<Ev3ButtonEvent*>(sample);
            asEvent->button(kind);
            asEvent->pressed(pressed);
            std::cout << buttonStr[kind] << " is " << (pressed ? "pressed" : "released") << '\n';
            myWriter->write(sample);
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
