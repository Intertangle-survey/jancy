Full Property Declaration
=========================

A full property declaration looks a lot like a declaration for a class. It implicitly opens a namespace and allows for overloaded setters, member fields, helper methods, constructors/destructors etc.

.. code-block:: none

	property g_prop
	{
	    int m_x = 5; // member field with in-place initializer

	    int get ()
	    {
	        return m_x;
	    }

	    set (int x)
	    {
	        m_x = x;
	        update ();
	    }

	    set (double x); // overloaded setter
	    update (); // helper method
	}

A body of a method can be placed on the right (Java-style) or outside (C++-style).
