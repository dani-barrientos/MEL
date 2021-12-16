#pragma once

#include <core/Exception.h>
namespace core
{
	class FOUNDATION_API ProcessException : public Exception
	{
	public:
		ProcessException(const char *msg,...);
		ProcessException( const ProcessException& );
	protected:
		//!@override Exception
		virtual void throwSame() const;

		//!@override Exception
		void _onClone( Exception** exc ) const;

	};
}