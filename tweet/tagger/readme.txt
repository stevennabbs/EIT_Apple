How to compile :
    Just do a simple make and it should be ok.

How to run :
    As simple as :
	./tagger input.txt output.txt

    The input.txt parameter is mandatory and point to the corpus file to load,
    while the output.txt is optional and specify the name of the file to write
    when your work is saved. It default to the same name as the input file.

How to use :

    Ctrl-Q / Ctrl-C : Exit the program (ask for saving before)
    Ctrl-S          : Save
    Ctrl-L          : Force screen redrawing

    Space : Clear current label and move to next unlabelled sample.

    Movement keys :
	J         : Go to the next sample.
	j / Down  : Go to the next unlabelled sample.
	K         : Go to the previous sample
	k / Up    : Go to the previous unlabelled sample.

    Labelling keys :
	h / Left  : Select previous label
	l / Right : Select next label
	q         : Select '???' label
	s         : Select 'pos' label
	d         : Select 'neu' label
	f         : Select 'neg' label
	g         : Select 'irr' label

