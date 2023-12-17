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

#include "ev3sensorsubscriber.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;

Ev3SensorSubscriber::Ev3SensorSubscriber() 
    : mySensorEventType(new Ev3SensorEventPubSubType()), /* object managed by TypeSupport class */
      myButtonEventType(new Ev3ButtonEventPubSubType())  /* object managed by TypeSupport class */
{
    DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;

#ifdef REMOTE_PEER
    // eprosima::fastrtps::rtps::Locator_t locator;
    // eprosima::fastrtps::rtps::IPLocator::setIPv4(locator, 0, 0, 0, 0);
    // locator.port = 7412;
    // qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);    

    // configure an initial peer on host 192.168.50.244.
    // The port number corresponds to the well-known port for metatraffic unicast
    // on participant ID `1` and domain `0`.
    // eprosima::fastrtps::rtps::Locator_t initial_peer;
    // eprosima::fastrtps::rtps::IPLocator::setIPv4(initial_peer, "192.168.50.244");
    // initial_peer.port = 7412;
    // qos.wire_protocol().builtin.initialPeersList.push_back(initial_peer);
#endif

    myParticipant = DomainParticipantFactory::get_instance()->create_participant(myDomain, qos);
    if (myParticipant == nullptr) {
        throw std::runtime_error("Failed creating domain participant");
    }  

    mySensorEventType.register_type(myParticipant);
    myButtonEventType.register_type(myParticipant);

    mySubscriber = myParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (mySubscriber == nullptr) {
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating domain subscriber");
    }

    mySensorEventTopic = myParticipant->create_topic("ev3_sensor_event_topic", myButtonEventType.get_type_name(), 
                                                      TOPIC_QOS_DEFAULT);    
    if (mySensorEventTopic == nullptr)  {
        myParticipant->delete_subscriber(mySubscriber);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event topic");
    }

    DataReaderQos rqos = mySubscriber->get_default_datareader_qos();
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.liveliness().lease_duration = 5;
    rqos.liveliness().announcement_period = 1;
    rqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;

    myReader = mySubscriber->create_datareader(mySensorEventTopic, rqos, &myListener);
    if (myReader == nullptr) {
        myParticipant->delete_topic(mySensorEventTopic);
        myParticipant->delete_subscriber(mySubscriber);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 data reader");
    }
}

Ev3SensorSubscriber::~Ev3SensorSubscriber()
{
    assert(mySubscriber != nullptr);
    mySubscriber->delete_datareader(myReader);
    assert(myParticipant != nullptr);
    myParticipant->delete_subscriber(mySubscriber);
    myParticipant->delete_topic(mySensorEventTopic);
    DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
}

void Ev3SensorSubscriber::SubListener::on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1) {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    } else if (info.current_count_change == -1) {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    } else {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

static std::map<enum Ev3Button, std::string> buttonStr = {
    { Back, "Back" },
    { Up, "Up" },
    { Down, "Down" },
    { Left, "Left" },
    { Right, "Right" },
    { Enter, "Enter" }
};

void Ev3SensorSubscriber::SubListener::on_data_available(DataReader* reader)
{
    SampleInfo info;
    Ev3ButtonEvent event;

    if (reader->take_next_sample(&event, &info) == ReturnCode_t::RETCODE_OK) {
        if (info.instance_state != ALIVE_INSTANCE_STATE) {
            std::cout << "instance not alive\n";
        } else  {
            samples_++;
            std::cout << "Button " << buttonStr[event.button()] << " is " << (event.pressed() ? "pressed" : "released") <<  std::endl;
        }
    }
}
