#include "global.hpp"
#include <memory>
#include <algorithm>

using namespace std;
using namespace Template;
using namespace Log;

namespace GL
{
   void ContextListener::init()
   {
      if (!dead_manager)
         ContextManager::get().register_listener(this);
   }

   void ContextListener::deinit()
   {
      if (!dead_manager)
         ContextManager::get().unregister_listener(this);
   }

   void ContextListener::register_dependency(ContextListener *listener)
   {
      if (!listener || dead_manager)
         return;

      ContextManager::get().register_dependency(this, listener);
   }

   void ContextListener::unregister_dependency(ContextListener *listener)
   {
      if (!listener || dead_manager)
         return;

      ContextManager::get().unregister_dependency(this, listener);
   }

   static unique_ptr<ContextManager> manager;

   ContextManager& ContextManager::get()
   {
      if (manager)
         return *manager;
      else
      {
         log("Creating context manager.");
         manager = unique_ptr<ContextManager>(new ContextManager);
         return *manager;
      }
   }

   void ContextManager::register_listener(ContextListener *listener)
   {
#ifdef GL_DEBUG
      cerr << "Registering listener: " <<
         static_cast<void*>(listener) << endl;
#endif

      auto pred = [listener](const shared_ptr<ListenerState>& state) {
         return state->listener == listener;
      };

      if (find_if(begin(listeners), end(listeners), pred) != end(listeners))
         return;

      auto state = make_shared<ListenerState>();
      state->listener = listener;
      state->id = context_id++;
      if (alive)
         state->reset_chain();
      listeners.push_back(move(state));
   }

   void ContextManager::register_dependency(ContextListener *master, ContextListener *slave)
   {
      if (!master || !slave)
         return;

      register_listener(master);

#ifdef GL_DEBUG
      cerr << "Registering dependency: " <<
         static_cast<void*>(master) << " < " << static_cast<void*>(slave) << endl;
#endif

      auto pred_master = [master](const shared_ptr<ListenerState>& state) {
         return state->listener == master;
      };

      auto pred_slave  = [slave](const shared_ptr<ListenerState>& state) {
         return state->listener == slave;
      };

      auto& itr_master = find_if_or_throw(listeners, pred_master);
      auto& itr_slave  = find_if_or_throw(listeners, pred_slave);
      itr_master->dependencies.push_back(itr_slave);
      itr_slave->dependers.push_back(itr_master);

      if (alive)
      {
         itr_master->reset_chain();
         itr_slave->reset_chain();
      }
   }

   void ContextManager::unregister_dependency(ContextListener *master_ptr, ContextListener *slave_ptr)
   {
      if (!master_ptr || !slave_ptr)
         return;

#ifdef GL_DEBUG
      cerr << "Unregistering dependency: " <<
         static_cast<void*>(master_ptr) << " < " << static_cast<void*>(slave_ptr) << endl;
#endif

      auto pred_master = [master_ptr](const shared_ptr<ListenerState>& state) {
         return state->listener == master_ptr;
      };

      auto pred_slave  = [slave_ptr](const shared_ptr<ListenerState>& state) {
         return state->listener == slave_ptr;
      };

      auto& itr_master = find_if_or_throw(listeners, pred_master);
      auto& itr_slave  = find_if_or_throw(listeners, pred_slave);

      auto pred_master_erase = [&itr_master](const weak_ptr<ListenerState>& state) {
         return state.lock() == itr_master;
      };

      auto pred_slave_erase = [&itr_slave](const weak_ptr<ListenerState>& state) {
         return state.lock() == itr_slave;
      };

      erase_if_all(itr_master->dependencies, pred_slave_erase);
      erase_if_all(itr_slave->dependers, pred_master_erase);
   }

   void ContextManager::unregister_listener(const ContextListener *listener)
   {
#ifdef GL_DEBUG
      cerr << "Unregistering listener: " <<
         static_cast<const void*>(listener) << endl;
#endif

      auto pred = [listener](const weak_ptr<ListenerState>& state) {
         return shared_ptr<ListenerState>(state)->listener == listener;
      };

      auto& itr = find_if_or_throw(listeners, pred);
      auto itr_pred = [&itr](const weak_ptr<ListenerState>& state) {
         return state.lock() == itr;
      };

      for (auto& parent_listener : itr->dependers)
         erase_if_all(parent_listener.lock()->dependencies, itr_pred);
      for (auto& child_listener : itr->dependencies)
         erase_if_all(child_listener.lock()->dependers, itr_pred);

      if (itr->signaled)
      {
         itr->signaled = false;
         itr->listener->destroyed();
      }

      erase_all(listeners, itr);
   }

   void ContextManager::notify_reset()
   {
      alive = true;
      for (auto& state : listeners)
         state->reset_chain();
   }

   void ContextManager::notify_destroyed()
   {
      for (auto& state : listeners)
         state->destroy_chain();
      alive = false;
   }

   void ContextManager::ListenerState::reset_chain()
   {
      if (!signaled)
      {
         signaled = true;
         for (auto state : dependencies)
            state.lock()->reset_chain();

#ifdef GL_DEBUG
         cerr << "Resetting: " <<
            static_cast<void*>(listener) << endl;
#endif
         listener->reset();
      }
   }

   void ContextManager::ListenerState::destroy_chain()
   {
      if (signaled)
      {
         signaled = false;
         for (auto state : dependers)
            state.lock()->destroy_chain();

#ifdef GL_DEBUG
         cerr << "Destroying: " <<
            static_cast<void*>(listener) << endl;
#endif
         listener->destroyed();
      }
   }

   ContextManager::~ContextManager()
   {
      for (auto& listener : listeners)
         listener->listener->set_dead_manager();

      log("Context manager tearing down!");
   }
}

