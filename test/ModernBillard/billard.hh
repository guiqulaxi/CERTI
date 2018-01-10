#ifndef BILLARD_H
#define BILLARD_H

#include <RTI/NullFederateAmbassador.h>
#include <RTI/RTI1516.h>

#include <RTI/certiLogicalTimeFactory.h>

#include <vector>

#include "ball.h"

using namespace rti1516e;

class Billard : public NullFederateAmbassador {
public:
    Billard(RTIambassador& ambassador, const std::wstring& federation_name, const std::wstring& federate_name);

    virtual ~Billard() = default;

    bool isCreator() const;
    bool hasSynchronizationPending(const std::wstring& label);
    void enableCollisions();

    void createOrJoin();

    void pause(const std::wstring& label);
    void synchronize(const std::wstring& label);

    void publishAndSubscribe();

    void enableTimeRegulation();

    void tick();

    void init();

    void declare();

    void step();

    void sendCollision(const Ball& other, const rti1516e::LogicalTime& time);

    void sendNewPosition(const rti1516e::LogicalTime& time);

    void announceSynchronizationPoint(
        const std::wstring& label,
        const rti1516e::VariableLengthData& theUserSuppliedTag) throw(FederateInternalError) override;

    void waitForAnnounce(const std::wstring& label);

    virtual void federationSynchronized(const std::wstring& label,
                                        const FederateHandleSet& failedToSyncSet) throw(FederateInternalError) override;

    void waitForSynchronization(const std::wstring& label);

    virtual void timeAdvanceGrant(const rti1516e::LogicalTime& theTime) throw(FederateInternalError) override;

    void waitForTimeAdvanceGrant();

    void discoverObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                                rti1516e::ObjectClassHandle theObjectClass,
                                const std::wstring& theObjectInstanceName) throw(FederateInternalError) override;

    void discoverObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                                rti1516e::ObjectClassHandle theObjectClass,
                                const std::wstring& theObjectInstanceName,
                                rti1516e::FederateHandle producingFederate) throw(FederateInternalError) override;

    void receiveInteraction(rti1516e::InteractionClassHandle theInteraction,
                            const ParameterHandleValueMap& theParameterValues,
                            const rti1516e::VariableLengthData& theUserSuppliedTag,
                            rti1516e::OrderType sentOrder,
                            rti1516e::TransportationType theType,
                            const rti1516e::LogicalTime& theTime,
                            rti1516e::OrderType receivedOrder,
                            rti1516e::MessageRetractionHandle theHandle,
                            rti1516e::SupplementalReceiveInfo theReceiveInfo) throw(FederateInternalError) override;

    void receiveInteraction(rti1516e::InteractionClassHandle theInteraction,
                            const ParameterHandleValueMap& theParameterValues,
                            const rti1516e::VariableLengthData& theUserSuppliedTag,
                            rti1516e::OrderType sentOrder,
                            rti1516e::TransportationType theType,
                            const rti1516e::LogicalTime& theTime,
                            rti1516e::OrderType receivedOrder,
                            rti1516e::SupplementalReceiveInfo theReceiveInfo) throw(FederateInternalError) override;

    void receiveInteraction(rti1516e::InteractionClassHandle theInteraction,
                            const ParameterHandleValueMap& theParameterValues,
                            const rti1516e::VariableLengthData& theUserSuppliedTag,
                            rti1516e::OrderType sentOrder,
                            rti1516e::TransportationType theType,
                            rti1516e::SupplementalReceiveInfo theReceiveInfo) throw(FederateInternalError) override;

    void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject,
                                const AttributeHandleValueMap& theAttributeValues,
                                const rti1516e::VariableLengthData& theUserSuppliedTag,
                                rti1516e::OrderType sentOrder,
                                rti1516e::TransportationType theType,
                                const rti1516e::LogicalTime& theTime,
                                rti1516e::OrderType receivedOrder,
                                rti1516e::MessageRetractionHandle theHandle,
                                rti1516e::SupplementalReflectInfo theReflectInfo) throw(FederateInternalError) override;

    void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject,
                                const AttributeHandleValueMap& theAttributeValues,
                                const rti1516e::VariableLengthData& theUserSuppliedTag,
                                rti1516e::OrderType sentOrder,
                                rti1516e::TransportationType theType,
                                const rti1516e::LogicalTime& theTime,
                                rti1516e::OrderType receivedOrder,
                                rti1516e::SupplementalReflectInfo theReflectInfo) throw(FederateInternalError) override;

    void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject,
                                const AttributeHandleValueMap& theAttributeValues,
                                const rti1516e::VariableLengthData& theUserSuppliedTag,
                                rti1516e::OrderType sentOrder,
                                rti1516e::TransportationType theType,
                                rti1516e::SupplementalReflectInfo theReflectInfo) throw(FederateInternalError) override;

private:
    void show_sync_points() const;

    RTIambassador& my_ambassador;

    const std::wstring& my_federation_name;
    const std::wstring& my_federate_name;

    bool has_created{false};
    bool has_collision_enabled{false};

    rti1516e::FederateHandle my_handle;

    std::vector<std::wstring> my_synchronization_points;

    Board my_board{};

    Ball my_ball{};
    std::vector<Ball> my_other_balls{};

    //     rti1516e::HLAfloat64TimeFactory my_time_factory{};
    std::unique_ptr<LogicalTimeFactory> my_time_factory{
        rti1516e::LogicalTimeFactoryFactory().makeLogicalTimeFactory(L"HLAfloat64Time")};

    std::unique_ptr<rti1516e::LogicalTimeInterval> my_time_interval{my_time_factory->makeEpsilon()};

    std::unique_ptr<rti1516e::LogicalTime> my_local_time{my_time_factory->makeInitial()};
    std::unique_ptr<rti1516e::LogicalTime> my_last_granted_time{my_time_factory->makeInitial()};
};

#endif // BILLARD_H