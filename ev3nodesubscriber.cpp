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

#include "ev3nodesubscriber.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;


Ev3NodeSubscriber::Ev3NodeSubscriber()
    : myMoveCommandType(new Ev3MoveCommandPubSubType()) /* object managed by TypeSupport class */
{
    DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;
        
    eprosima::fastdds::dds::StatusMask mask;
    mask = StatusMask::all();
    mask.set(9, false);      //don't call data_on_readers()

    // eprosima::fastrtps::rtps::Locator_t locator;
    // eprosima::fastrtps::rtps::IPLocator::setIPv4(locator, 192, 168, 50, 244);
    //locator.port = 7412;
    // qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);    

    // The initial peer address.
    // eprosima::fastrtps::rtps::Locator_t initial_peer;
    // eprosima::fastrtps::rtps::IPLocator::setIPv4(initial_peer, 192, 168, 50, 178);
    // qos.wire_protocol().builtin.initialPeersList.push_back(initial_peer);

    myParticipant = DomainParticipantFactory::get_instance()->create_participant(myDomain, qos, &myDomainListener, mask);
    if (myParticipant == nullptr) {
        throw std::runtime_error("Failed creating domain participant");
    }  

    try {
        myMoveCommandType.register_type(myParticipant);

        mySubscriber = myParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (mySubscriber == nullptr) {
            throw std::runtime_error("Failed creating domain subscriber");
        }

        myMoveCommandTopic = myParticipant->create_topic("ev3_move_command_topic", myMoveCommandType.get_type_name(),
                                                         TOPIC_QOS_DEFAULT);
        if (myMoveCommandTopic == nullptr)  {
            throw std::runtime_error("Failed creating EV3 move command topic");
        }

        DataReaderQos rqos = mySubscriber->get_default_datareader_qos();
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

        myReader = mySubscriber->create_datareader(myMoveCommandTopic, rqos, &myListener);
        if (myReader == nullptr) {
            throw std::runtime_error("Failed creating EV3 move data reader");
        }
    } catch (const std::exception& e) {
        cleanup_dds_objects();
        throw;
    }
}

void Ev3NodeSubscriber::cleanup_dds_objects()
{
    assert(myParticipant != nullptr);        
    if (myReader != nullptr) {
        assert(mySubscriber != nullptr);
        mySubscriber->delete_datareader(myReader);
    }
    if (myMoveCommandTopic != nullptr) {        
        myParticipant->delete_topic(myMoveCommandTopic);
    }
    if (mySubscriber != nullptr) {
        myParticipant->delete_subscriber(mySubscriber);
    }
    DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
}

Ev3NodeSubscriber::~Ev3NodeSubscriber()
{
    cleanup_dds_objects();
}

void Ev3NodeSubscriber::SubListener::on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1) {
        matched_ = info.total_count;
        std::cout << "EV3 move subscriber matched." << std::endl;
    } else if (info.current_count_change == -1) {
        matched_ = info.total_count;
        std::cout << "EV3 move subscriber unmatched." << std::endl;
    } else {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void Ev3NodeSubscriber::SubListener::on_data_available(DataReader* reader)
{
    SampleInfo info;
    Ev3MoveCommand cmd;

    if (reader->take_next_sample(&cmd, &info) == ReturnCode_t::RETCODE_OK) {
        if (info.instance_state != ALIVE_INSTANCE_STATE) {
            std::cout << "instance not alive\n";
        } else  {
            if (myCmdListener != nullptr) {
                myCmdListener->on_move_command(cmd);
            } else {
                std::cout << "Null cmd listener\n";
            }
        }
    }
}
