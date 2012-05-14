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

#ifndef APPLICATION_H
#define APPLICATION_H

namespace icinga {

class Component;

DEFINE_EXCEPTION_CLASS(ComponentLoadException);

class I2_BASE_API Application : public Object {
private:
	bool m_ShuttingDown;
	ConfigHive::Ptr m_ConfigHive;
	map< string, shared_ptr<Component> > m_Components;
	vector<string> m_Arguments;
	bool m_Debugging;

protected:
	void RunEventLoop(void);
	string GetExeDirectory(void) const;

public:
	typedef shared_ptr<Application> Ptr;
	typedef weak_ptr<Application> WeakPtr;

	static Application::Ptr Instance;

	Application(void);
	~Application(void);

	virtual int Main(const vector<string>& args) = 0;

	void SetArguments(const vector<string>& arguments);
	const vector<string>& GetArguments(void) const;

	void Shutdown(void);

	static void Log(string message);

	ConfigHive::Ptr GetConfigHive(void) const;

	shared_ptr<Component> LoadComponent(const string& path,
	    const ConfigObject::Ptr& componentConfig);
	void RegisterComponent(shared_ptr<Component> component);
	void UnregisterComponent(shared_ptr<Component> component);
	shared_ptr<Component> GetComponent(const string& name);
	void AddComponentSearchDir(const string& componentDirectory);

	bool IsDebugging(void) const;
};

int I2_EXPORT RunApplication(int argc, char **argv, Application::Ptr instance);

}

#endif /* APPLICATION_H */
