# ustar
Basic ustar archive parser and writer

It's very basic. Running make will yield a sample program (ustar) with the following command line arguments: 

./ustar input_archive output_archive 

The program will parse input_archive, create a full archive structure from it, and then reserialize it into the output_archive file.

It has basic error checking, so most corrupt archives shouldn't pass inspection.
