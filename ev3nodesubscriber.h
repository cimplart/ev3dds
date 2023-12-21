#ifndef EV3NODESUBSCRIBER_H
#define EV3NODESUBSCRIBER_H

#pragma once

#include "ev3dataPubSubTypes.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include "ev3data.h"

class Ev3NodeSubscriber
{
public:
    Ev3NodeSubscriber();
    virtual ~Ev3NodeSubscriber();

    class CommandListener
    {
        public:
        virtual void on_move_command(const Ev3MoveCommand& cmd) = 0;
    };

    void set_listener(CommandListener* l)
    {
        myListener.set_cmd_listener(l);
    }

private:
    static constexpr eprosima::fastdds::dds::DomainId_t myDomain = 0;
    eprosima::fastdds::dds::DomainParticipant* myParticipant = nullptr;
    eprosima::fastdds::dds::Subscriber* mySubscriber = nullptr;
    eprosima::fastdds::dds::Topic* myMoveCommandTopic = nullptr;
    eprosima::fastdds::dds::DataReader* myReader = nullptr;
    eprosima::fastdds::dds::TypeSupport myMoveCommandType;

    class MyDomainListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
        virtual void on_participant_discovery(eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)  {
                std::cout << "New participant discovered" << std::endl;
            } else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
                    info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)  {
                std::cout << "New participant lost" << std::endl;
            }
        }        

        virtual void on_subscriber_discovery(
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER)  {
                std::cout << "New subscriber discovered" << std::endl;
            } else if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER)   {
                std::cout << "New subscriber lost" << std::endl;
            }
        }        
    } myDomainListener;

    using DomainParticipant = eprosima::fastdds::dds::DomainParticipant;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:
        SubListener()  : matched_(0)
        {
        }

        ~SubListener() override
        {
        }

        void set_cmd_listener(CommandListener *l)
        {
            myCmdListener = l;
        }

        void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(eprosima::fastdds::dds::DataReader* reader,
                                     const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_requested_deadline_missed(eprosima::fastdds::dds::DataReader* reader,
                    const eprosima::fastrtps::RequestedDeadlineMissedStatus& info) override
        {
            static_cast<void>(reader);
            static_cast<void>(info);
            std::cout << "Some data was not received on time" << std::endl;
        }                                     

        void on_sample_rejected(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastrtps::SampleRejectedStatus& info) override
        {
            static_cast<void>(reader);
            static_cast<void>(info);
            std::cout << "A received data sample was rejected" << std::endl;
        }

        void on_requested_incompatible_qos(
                eprosima::fastdds::dds::DataReader* /*reader*/,
                const eprosima::fastdds::dds::RequestedIncompatibleQosStatus& info) override
        {
            std::cout << "Found a remote Topic with incompatible QoS (QoS ID: " << info.last_policy_id <<
                ")" << std::endl;
        }

        void on_sample_lost(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SampleLostStatus& status) override
        {
            static_cast<void>(reader);
            static_cast<void>(status);
            std::cout << "A data sample was lost and will not be received" << std::endl;
        }

        int matched_;
        CommandListener *myCmdListener = nullptr;
    }  myListener;
   
    void cleanup_dds_objects();
};

#endif