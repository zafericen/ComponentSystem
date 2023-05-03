#ifndef SYSTEM_H
#define SYSTEM_H

namespace CECS
{
	class ISystem
	{
	public:
		ISystem() = default;

		virtual ~ISystem() = default;

		virtual void initilize()
		{
		}

		virtual void terminate()
		{
		}

		virtual void update() = 0;
	};
}

#endif