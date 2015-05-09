Categorization-And-Learning-Module API
--------------------------------------

### Introduction

The CALM-API is a C++ code library for the Categorization-And-Learning-Module neural architecture, a neural network algorithm originally developed by [Murre](http://www.murre.com/), Phaf and Wolters. The API implementation has a long history, with its origins in the now deceased MacOS 9 application, called Poise. The current instantiation is purely UNIX-based, providing the necessary routines for the user to access data and methods of the CALM neural network, with support for both online and offline learning. The API library is designed in OOP-style, allowing access via an encapsulated interface. Employing calls to an instantiated API library , the user can flexibly design a set of routines to run a simulation. Example simulation code is included.

### Installation

To compile the library, navigate to the subdirectory `calmlib/` and just issue a `make` command. The Makefile located in `exec/` will compile an executable based on the library and a set of simulation code files, which you will need to specify in the Makefile. The `bin/` directory listed in the Makefile, where the compiled executables will be installed, needs to be modified to reflect the user's own home directory. If you are using Xcode, you will also need to modify the working directory in the Custom Build Command Settings of the "CALMLib" and "CALMApp" targets. Included in the package are `Main.cpp` and `MultiSequence.cpp`, both of which do not need any modifications. The first file creates a command-line executable that can be called with a set of run-time options, described in `Main.cpp` as well as the man page. The second file is a support file for training the network with a set of offline sequences. The files `SampleOffline.cpp`, `SampleOnline.cpp`, `SampleFeedback.cpp`, and `MultiSeqSample.cpp` provide extensively commented illustrative examples. The latter file demonstrates offline training of multiple sequences, with the option to not only use feedback signals, but also to grow and prune CALM modules.

In addition, sample network specification, parameter and pattern files are also included in the directory `simulations/`. The `multi/` subdirectory provides the necessary files to run the sample multiple sequence training procedure.

### File Format

Specifications for the network architecture, parameters, and input patterns are defined in text files with appropriate suffixes. These files should be in the same directory and share the same base name. The MultiSequence class requires a set of pattern files, each defining a single sequence. These filenames should conform to the format "`basename-``index``.pat`" with `index` a number starting at 0. For example, `multi-0.pat`, `multi-1.pat`, and so on. The following extensions are required:
 `.net`: specifications for the network
 `.par`: parameters for the network
 `.pat`: training patterns
 `.fb`: feedback signals
 `⇒` Network specification files should be in the following format:

    # comments follow a hedge. The API will ignore the rest of the line.
    # first list the number of CALM modules. Here, we only have one.
    1
    # then list the number of input modules (yes, you can have more than one).
    1
    # set up each module in turn, starting with the input modules
    # the input module has name "pat", is an "input" module, containing 2 nodes
    pat input 2
    # the output module (which is just a plain CALM module), is named "out" and has 2 nodes
    # possible other module types are 'map' and 'fb' for the self-organizing CALMMap and the
    # feedback module, respectively
    out calm 2
    # define connections: "out" receives activations from "pat". It's the only connection.
    # the other possible connection type is 'delay', which also requires the delay value
    # of the connection, which should be an integer greater than 0.
    out 1 pat normal

`⇒` Parameter files should be in the following format:

    # the values for the fixed intra-modular connections
    0.5      # UP
    -0.2     # DOWN
    -10.0    # CROSS
    -1.0     # FLAT
    -0.6     # HIGH
    0.4      # LOW
    1.0      # AE
    0.1      # ER
    # initial value for the learning weight
    0.6      # INITWT
    # activation value criteria used to determine convergence of a module (i.e. is there a winner?)
    0.1      # LOWCRIT
    0.1      # HIGHCRIT
    # parameters for learning and activation rule (see CALM documentation)
    0.05     # activation rule decay-parameter
    1.0      # maximum weight value (K)
    0.0      # minimum weight value
    1.0      # learning rule L-parameter
    0.0001   # base learning-rate
    0.005    # virtual weight from the E-node to the learning rate parameter
    # Gaussian weight update parameters
    0.5      # G_L
    0.25     # G_W
    # feedback-related parameters
    1.0      # learning-rate amplification for feedback signals
    1.0      # feedback activation amplification
    # CALMMap-related parameter (unused)
    0.1      # SIGMA
    # parameters to control growing and shrinking
    50.0     # percentual difference between maximum and next-largest potential needed to grow
    0.0001   # minimum potential before node is marked for pruning
    # below are obsolete parameters (leave as is)
    0        # U_L
    8.8      # A
    10       # B

`⇒` Pattern files follow the following format. The first line indicates the number of patterns in the file (this should be the same for each input module), with subsequent lines specifing the pattern set for each input module, in the same order in which the input modules have been declared in the network specification file). For example:

    # 2 patterns
    2
    # set of patterns for first input module
    1 1 0 0 1 0
    1 1 0 1 1 1
    # for second input module
    0 0 1 1 1 0
    0 1 1 0 1 1

`⇒` A file containing feedback signals follows the same format as the patterns file. Instead of patterns, the indices of the nodes of the feedback module designated to win the competition are listed. The order of indices follows the order of patterns in the patterns file.

### Usage

In order to program custom simulation code, the user must adhere to the following API usage guidelines, assuming the provided `Main.cpp` file will be used. This file already creates a `CALMAPI` instance, which must be externally declared in the custom simulation file:

``` 
extern CALMAPI *gCALMAPI;
```

The API is itself a class called `CALMAPI` and which must be instantiated (as is done inside `Main.cpp`) before any API call is made. In addition, two functions should be defined, which are declared as:

``` 
bool InitNetwork( void );
void DoSimulation( void );
```

`⇒` In the `InitNetwork()` procedure, calls to set up the network architecture plus parameters should be made. 

Any console output from the library can be redirected to file by providing an initiated and opened `ofstream` instance, but by default it goes to `cout` so the call below does not have to be made.

``` 
gCALMAPI->SetCALMLog( &cout );
```

The next call loads the parameters for the CALM network. This call MUST precede the call to initialize the network. The API library returns an error value if the file could not be loaded. The return code must be checked to allow for safely aborting the simulation.

``` 
if ( gCALMAPI->CALMLoadParameters() != kNoErr ) return false;
```

The next call allocates the network objects and initializes it using the given network specification file. The API library returns an error value if it could not allocate or initialize the network. The return code must be checked to allow for safely aborting the simulation.

``` 
gCALMAPI->CALMSetupNetwork( &calmErr );
if ( calmErr != kNoErr ) return false;
```

If the simulation is in **offline** mode, patterns should be loaded after initializing the network:

``` 
if ( gCALMAPI->CALMLoadPatterns() != kNoErr ) return false;
```

In case a feedback module is used, the feedback signals corresponding to the patterns should also be loaded:

``` 
if ( gCALMAPI->CALMLoadFeedback() != kNoErr ) return false;
```

If the simulation is in **online** mode, the API should be informed that it will be used in online fashion with the following call, which will allocate an internal buffer that is a vector with the length corresponding to the sum of the number of nodes of all input modules:

``` 
gCALMAPI->CALMOnlinePatterns();
```

In case a feedback module is used, feedback signals must be set manually before training a pattern. The feedback signal is the index of the R node that is designated to represent the current pattern:

``` 
gCALMAPI->CALMSetFeedback(i);
```

Optionally display a summary of the network settings and patterns with the following calls:

``` 
gCALMAPI->CALMShow();
gCALMAPI->CALMShowPatterns();
```

`⇒` The `DoSimulation()` procedure will be responsible for the actual execution of a simulation. Observe the following key steps:

If training in **offline** mode, make sure patterns were loaded. In addition, if the patterns should be presented in permuted order, issue the call:

``` 
if ( gCALMAPI->CALMGetOrder() == kPermuted ) gCALMAPI->CALMPermutePatterns();
```

Training and testing then proceed using the API built-in routines:

``` 
gCALMAPI->CALMTrainFile( epoch );
```

and

``` 
gCALMAPI->CALMTestFile( run );
```

For the **online** training mode, the user must provide the API library with input patterns. To set a custom pattern, use for example:

``` 
for ( int i = 0; i < gCALMAPI->CALMGetInputLen(); i++ )
    gCALMAPI->CALMSetOnlineInput( i, PseudoRNG() );
```

which just sets a input vector with random values between 0.0 and 1.0. Training and testing then require calling:

``` 
gCALMAPI->CALMTrainSingle( epoch );
```

and

``` 
gCALMAPI->CALMTestSingle( run );
```

Before and during simulation, it may be desirable to reset particular network values. Values that can be reset are weights, activations, winning node information, and time-delay activations. This can be called using a bitwise or operation of the appropriate constants (see CALMGlobal.h). Before commencing a simulation, call the following function to set the weights to zero:

``` 
gCALMAPI->CALMReset( O_WT | O_TIME | O_WIN );
```

`⇒` Other useful API calls:

Naturally, it would desirable to store the trained weights for future analysis or re-use. The following snippet saves the weights from the CALM network to the file `final.wts` in the same directory as the network specification file (the file extension is added by the API):

``` 
gCALMAPI->CALMSaveWeights( "final" );
```

The saved weights can then be re-loading using:

``` 
gCALMAPI->CALMLoadWeights( "final" );
```

The CALM implementation of CALM-API uses a preliminary method for growing CALM modules and pruning inactive R-V node pairs. After a simulation, the final network architecture may have differently sized modules. In such a case, saving the new network configuration would be recommended. The following API-call saves the new network architecture to the file `new-net.net` in the same directory as the network specification file (the file extension is added by the API):

``` 
gCALMAPI->CALMWriteNetwork( &calmErr, "new-net" );
if ( calmErr != kNoErr ){ /* handle error */ }
```

To reload the original network for a new run, the following call can be used. After re-loading a network, the patters MUST be re-loaded as well:

``` 
gCALMAPI->CALMSetupNetwork( &calmErr, gCALMAPI->CALMGetBasename() );
if ( gCALMAPI->CALMLoadPatterns() != kNoErr ){ /* handle error */ };
```

As mentioned earlier, the current CALM implementation allows for growing and shrinking of modules. The method used involves keeping track of an R-nodes so-called "potential", which is a moving average variant of the change in activation of an R-node, modified by the E-node activation. When this potential drops below a given threshold, the corresponding R-V pair is pruned. When the maximum potential value exceeds the second-largest potential by a pre-defined percentage (indicating an off-balanced competition), a new R-V pair is added. (For more information, see the CALM documentation.)

The easiest way to use the growing/pruning algorithm, is to call

``` 
resized = gCALMAPI->CALMResizeModule();
```

after every number of epochs. This function checks if resizing is necessary and proceeds to do so if positive. Any growing or pruning is reported to console and a boolean for true is returned. The API contains calls to check if a module needs resizing and to manually resize a module to a given number of R-V pairs.

### Multiple Sequences

With the implementation of time-delay connections, the CALM network can be trained with sequential information. The `MultiSequence` class contains routines to train a given network using an ordered set of sequences. Compile an executable for the files `Main.cpp`, `MultiSequence.cpp` and `MultiSeqSample.cpp`, and run it with:

``` 
calm -r 1 -e 10 -i 100 -d simulations/multi -b king -v 0 -p 0
```

This will use the files in the `simulations/multi/` directory to set up a network for training a set of 10 sequences in incremental fashion, the order of which is defined by the indices of the pattern files. By default, the network allows for pruning and growing and it uses feedback signals, but these can be switched off by setting the appropriate defines on top of the `MultiSequence.cpp` file. The progress and final performance of the network is written to the standard console, as follows.

        1 [0: 0 1] [1: 1 1]
        2 [0: 0 1] [1: 1 1]
        3 [0: 0 1] [1: 1 1]
        4 [0: 0 1] [1: 1 1]
        5 [0: 0 2] [1: 1 1]
    d3 +1 -> 3
    agg +1 -> 3
        6 [0: 0 1] [1: 1 1]
        7 [0: 0 1] [1: 1 1]
        8 [0: 0 1] [1: 1 1]
        9 [0: 0 1] [1: 1 1]
       10 [0: 0 2] [1: 1 1]
       11 [0: 0 1] [1: 1 1]
       12 [0: 0 1] [1: 1 1]
       13 [0: 0 1] [1: 1 1]
       14 [0: 0 1] [1: 1 1]
       15 [0: 0 2] [1: 1 1]
       16 [0: 0 1] [1: 1 1]
       17 [0: 0 0] [1: 1 0]
    17
       18 [0: 0 0] [1: 1 0] [2: 1 10]
       19 [0: 0 1] [1: 1 1] [2: 1 10]

The multi-sequence training procedure starts off with two sequences and learns to separate each sequence by means of feedback signals. The first line shows the initial training progress of the first two sequences. In

    1 [0: 0 1] [1: 1 1]

the first number indicates the epoch. The first part between square brackets shows, first, the index of the sequence (0), the index of the node of the designated output module that won the competition during a testing phase, and the number of passes through the full sequence before the correct node in the output module won. Every 5 epochs (can be changed by modifying the `GROWCHECK` define in `MultiSequence.cpp`) modules sizes are adapted if necessary. Initially, all modules have 2 R-V pairs (but the `simulations/multi/` directory also contains a network specification file for simulations without growing and pruning). In the console read-out, module sizes for "d3" and "agg" are increased with one R-V pair (growth and shrinkage always occurs one R-V pair at a time). After the 17th epoch, no training was necessary, and a new sequence was added. For more information on the code, consult the comments in the appropriate files. For more information about the simulation, read the [journal paper](https://www.dropbox.com/s/l3sxy83ht5011vp/ieee.pdf).

It is recommended to study the example code files in the `simulations/` folder and the comments inside `CALM.cpp` and `CALM.h`. The file `Utilities.cpp` also contains a set of useful functions, including matrix allocation and disposal.
