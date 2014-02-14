//Copyright 2014, Intermodalics BVBA

#include <rtt/RTT.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/os/Time.hpp>

#include <jitter/typekit/Types.hpp>

using namespace RTT;
using namespace std;

/**
 * A jitter calculation service which can be loaded in a component.
 */
class JitterService : public RTT::Service , public RTT::base::ExecutableInterface {
public:
    JitterService(TaskContext* owner)
        : Service("jitter", owner),initialized_(false),prop_calc_stats(false),prop_expected_period(0.0),prop_window(0)
    {
        this->addPort("jitter",jitter_port).doc("The current jitter in ms");
        this->addPort("jitter_stats",port_stats).doc("Jitter statistics in ms");
        this->addProperty("expected_period",prop_expected_period).doc("(Only) if the owners period is zero, this can be set to a non-zero value in case the component is somehow triggered with a fixed timer. Leave it to zero otherwise!");
        this->addProperty("window",prop_window).doc("Samples to use for mean calculation, leave it to zero to use all");
        this->addProperty("enable_stats",prop_calc_stats).doc("Set to true to enable stats calculation");
        initialized_ = false;
        this->getOwnerExecutionEngine()->runFunction(this);
    }

    ~JitterService() {
        this->getOwnerExecutionEngine()->removeFunction(this);
    }

    bool execute() {
        if (this->getOwner()->getPeriod() != 0)
            prop_expected_period = this->getOwner()->getPeriod();
        if( this->getOwner()->isRunning() && initialized_ ){
            calculated_period_ = os::TimeService::Instance()->secondsSince(timestamp_);
            timestamp_ = os::TimeService::Instance()->getTicks();
            jitter_ = (calculated_period_ - prop_expected_period)*1000;//Let's use milliseconds
            jitter_port.write(jitter_);
            if(prop_calc_stats){
                stats_.current = jitter_;
                stats_.min = min(stats_.min,stats_.current);
                stats_.max = max(stats_.max,stats_.current);
                if(prop_window == 0 || jitter_buffer_.size() < prop_window){
                    stats_.avg = (stats_.current + stats_.nr_of_samples*stats_.avg)/(stats_.nr_of_samples+1);
                    stats_.nr_of_samples++;
                    if(prop_window != 0)
                        jitter_buffer_.push_back(stats_.current);
                } else{
                    //calculate the windowed mean
                    stats_.avg = (stats_.current + prop_window*stats_.avg - jitter_buffer_.front())/(prop_window);
                    stats_.nr_of_samples = prop_window;
                    //Remove the oldest value from the buffer and avg
                    jitter_buffer_.pop_front();
                    //Add the latest
                    jitter_buffer_.push_back(stats_.current);
                }
                port_stats.write(stats_);
            }
        } else{
            initialized_=true;
            timestamp_ = os::TimeService::Instance()->getTicks();
        }
        return true;
    }

    RTT::OutputPort<double> jitter_port;
    RTT::OutputPort<jitter::stats>  port_stats;
    jitter::stats stats_;
    os::TimeService::ticks timestamp_;
    bool initialized_,prop_calc_stats;
    double prop_expected_period,calculated_period_,jitter_;
    unsigned int prop_window;
    std::deque<double> jitter_buffer_;
};

/* For consistency reasons, it's better to name the
 * service the same as in the class above.
 */
ORO_SERVICE_NAMED_PLUGIN(JitterService, "jitter")
