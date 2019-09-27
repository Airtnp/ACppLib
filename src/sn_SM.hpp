#ifndef SN_SM_H
#define SN_SM_H

#include "sn_CommonHeader.h"

// TODO: add PDA
namespace sn_SM {
	// ref: https://github.com/eglimi/cppfsm
	namespace fsm {
		template <typename S, typename Tr>
		class FSM {
			
		public:
			using guard_t = std::function<bool()>;
			using action_t = std::function<void()>;
			struct Trans {
				S m_from;
				S m_to;
				Tr trigger;
				guard_t guard;
				action_t action;
			};

		private:
			S m_initial;
			S m_currentState;
			using transition_edge_t = std::vector<Trans>;
			using transition_t = std::map<S, transition_edge_t>;
			transition_t m_trans;
			
		public:

			FSM(S init)
				: m_initial(init), m_currentState(init), m_trans() {}
			void reset(S s) {
				m_currentState = m_initial;
			}
			template <typename It>
			void add_transitions(It start, It end) {
				for (It it = start; it != end; ++it)
					m_trans[(*it).from_state].push_bach(*it);
			}
			template <typename C>
			void add_transitions(C&& c) {
				add_transitions(std::begin(c), std::end(c));
			}
			void add_transitions(std::initializer_list<Trans>&& l) {
				add_transitions(std::begin(l), std::end(l));
			}
			void execute(Tr trigger) {
				const auto s = m_trans.find(m_currentState);
				const transition_edge_t sv = s->second;
				for (const auto& trans : sv) {
					if (trigger != trans.trigger)
						continue;
					if (trans.guard && !trans.guard())
						continue;
					if (trans.action)
						trans.action();
					m_currentState = trans.to_state;
					break;
				}
			}
			S state() const {
				return m_currentState;
			}
			bool is_initial() const {
				return m_currentState == m_initial;
			}
		};

		/*
		Usage:
			enum State { A, B };
			enum Trigger { Exec };
			FSM<State, Trigger> fsm{State::A};
			fsm.add_transitions(
				from, to, trigger, guard, action
			);		
		*/

	}

}



#endif