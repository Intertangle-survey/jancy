once
====

Jancy provides an elegant syntax for lazy initialization. Prefix the necessary piece of code with ``once`` and the compiler will generate a thread-safe wrapper. The latter will ensure that this code executes once per each program run.

.. code-block:: none

	foo ()
	{
	    once initialize ();

	    // ...
	}

If your lazy initialization requires more than a single statement, enclose the entire block of your initialization code in a compound statement:

.. code-block:: none

	foo ()
	{
	    once
	    {
	        initializeTables ();
	        initializeMaps ();
	        initializeIo ();

	        // ...
	    }

	    // ...
	}

Jancy also provides a way to run the lazy initialization once per thread. Use **threadlocal once** to achieve this:

.. code-block:: none

	foo ()
	{
	    threadlocal once initializeThread ();

	    // ...
	}
