MMDelta
=======

MMDelta is open source binary differention compression tool.
By allowing some mismatches during the matching process it's optimized for the creation of binary diff's of executables. 
It is based on the paper "Differential Compression of Executable Code" by Giovanni Motta, James Gustafson and Samson Chen published on the Data Compression Conference, 2007.

## Usage
Start the application using:
```
mmdelta (-e/-d) SOURCE1 SOURCE2 TARGET
	-e create a diff from SOURCE1 and SOURCE2 and save as TARGET
	-d decode the diff SOURCE2 using SOURCE1 and save as TARGET
```

## Copyright and license
Copyright 2015 Iwan Timmer. Distributed under the GNU GPL v3. For full terms see the LICENSE file
