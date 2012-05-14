/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012 Icinga Development Team (http://www.icinga.org/)        *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "i2-base.h"

using namespace icinga;

time_t Timer::NextCall;
Timer::CollectionType Timer::Timers;

/**
 * Constructor for the Timer class.
 */
Timer::Timer(void)
{
	m_Interval = 0;
}

/**
 * Retrieves when the next timer is due.
 *
 * @returns Time when the next timer is due.
 */
time_t Timer::GetNextCall(void)
{
	if (NextCall < time(NULL))
		Timer::RescheduleTimers();

	return NextCall;
}

/**
 * Reschedules all timers, thereby updating the NextCall
 * timestamp used by the GetNextCall() function.
 */
void Timer::RescheduleTimers(void)
{
	/* Make sure we wake up at least once every 30 seconds */
	NextCall = time(NULL) + 30;

	for (Timer::CollectionType::iterator i = Timers.begin(); i != Timers.end(); i++) {
		Timer::Ptr timer = i->lock();

		if (timer == NULL)
			continue;

		if (timer->m_Next < NextCall)
			NextCall = timer->m_Next;
	}
}

/**
 * Calls all expired timers and reschedules them.
 */
void Timer::CallExpiredTimers(void)
{
	time_t now;

	time(&now);

	Timer::CollectionType::iterator prev, i;
	for (i = Timers.begin(); i != Timers.end(); ) {
		Timer::Ptr timer = i->lock();

		prev = i;
		i++;

		if (!timer) {
			Timers.erase(prev);
			continue;
		}

		if (timer->m_Next <= now) {
			timer->Call();
			timer->Reschedule(now + timer->GetInterval());
		}
	}
}

/**
 * Calls this timer. Note: the timer delegate must not call
 * Disable() on any other timers than the timer that originally
 * invoked the delegate.
 */
void Timer::Call(void)
{
	TimerEventArgs tea;
	tea.Source = shared_from_this();
	tea.UserArgs = m_UserArgs;
	OnTimerExpired(tea);
}

/**
 * Sets the interval for this timer.
 *
 * @param interval The new interval.
 */
void Timer::SetInterval(unsigned int interval)
{
	m_Interval = interval;
}

/**
 * Retrieves the interval for this timer.
 *
 * @returns The interval.
 */
unsigned int Timer::GetInterval(void) const
{
	return m_Interval;
}

/**
 * Sets user arguments for the timer callback.
 *
 * @param userArgs The user arguments.
 */
void Timer::SetUserArgs(const EventArgs& userArgs)
{
	m_UserArgs = userArgs;
}

/**
 * Retrieves the user arguments for the timer callback.
 *
 * @returns The user arguments.
 */
EventArgs Timer::GetUserArgs(void) const
{
	return m_UserArgs;
}

/**
 * Registers the timer and starts processing events for it.
 */
void Timer::Start(void)
{
	Timers.push_back(static_pointer_cast<Timer>(shared_from_this()));

	Reschedule(time(NULL) + m_Interval);
}

/**
 * Unregisters the timer and stops processing events for it.
 */
void Timer::Stop(void)
{
	Timers.remove_if(weak_ptr_eq_raw<Timer>(this));
}

/**
 * Reschedules this timer.
 *
 * @param next The time when this timer should be called again.
 */
void Timer::Reschedule(time_t next)
{
	m_Next = next;
	RescheduleTimers();
}
