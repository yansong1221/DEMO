#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceTracker.h>
#include "demo/IGreetingService.h"
#include <iostream>
using namespace cppmicroservices;
namespace demo {
    class ConsumerActivator : public BundleActivator {
        //std::unique_ptr<ServiceTracker<demo::IGreetingService>> m_tracker;
    public:
        void Start(BundleContext context) override {

            m_context = context;

            {
                // Use your favorite thread library to synchronize member
                // variable access within this scope while registering
                // the service listener and performing our initial
                // dictionary service lookup since we
                // don't want to receive service events when looking up the
                // dictionary service, if one exists.
                // MutexLocker lock(&m_mutex);

                // Listen for events pertaining to dictionary services.
                m_context.AddServiceListener(
                    std::bind(&ConsumerActivator::ServiceChanged, this, std::placeholders::_1));

                // Query for any service references matching any language.
                auto refs =
                    context.GetServiceReferences<ITaskService>();

                // If we found any dictionary services, then just get
                // a reference to the first one so we can use it.
                if (!refs.empty()) {
                    m_ref = refs.front();
                    //m_dictionary = m_context.GetService(m_ref);
                }
            }
        }
        void Stop(BundleContext) override {
            //if (m_tracker) { m_tracker->Close(); m_tracker.reset(); }
        }

        void ServiceChanged(const ServiceEvent& event)
        {
            // Use your favorite thread library to synchronize this
            // method with the Start() method.
            // MutexLocker lock(&m_mutex);

            // If a dictionary service was registered, see if we
            // need one. If so, get a reference to it.
            if (event.GetType() & ServiceEvent::SERVICE_REGISTERED) {
                if (!m_ref) {
                    // Get a reference to the service object.
                    m_ref = ServiceReference<ITaskService>(event.GetServiceReference());
                    m_dictionary = m_context.GetService(m_ref);
                }
            }
            // If a dictionary service was unregistered, see if it
            // was the one we were using. If so, unget the service
            // and try to query to get another one.
            else if (event.GetType() & ServiceEvent::SERVICE_UNREGISTERING) {
                const ServiceReferenceBase eventRef   = event.GetServiceReference();
                const ServiceReferenceBase currentRef = m_ref;
                if (eventRef == currentRef) {
                    // Unget service object and null references.
                    m_ref = nullptr;
                    m_dictionary.reset();

                    // Query to see if we can get another service.
                    std::vector<ServiceReference<ITaskService>> refs;
                    try {
                        refs =
                            m_context.GetServiceReferences<ITaskService>();
                    }
                    catch (const std::invalid_argument& e) {
                        std::cout << e.what() << std::endl;
                    }

                    if (!refs.empty()) {
                        // Get a reference to the first service object.
                        m_ref = refs.front();
                        m_dictionary = m_context.GetService(m_ref);
                    }
                }
            }

        }
    private:
        BundleContext m_context;
        ServiceReference<ITaskService> m_ref;
        std::shared_ptr<ITaskService> m_dictionary;
    };
}
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(demo::ConsumerActivator)
