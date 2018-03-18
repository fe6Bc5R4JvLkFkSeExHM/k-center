This directory contains a collection of timestamped GPS coordinates collected using the Twitter API.
The dataset was split in chunk of 50Mo due to the limit imposed by github.
To uncompress it, use:
   cat twitter.tar.gz.parta* | tar -xzvf filename

Each line has the following format:
timestamp tab longitude space latitude
