Tool to Query Events from Eventful web API

Dependencies: libcurl, C++11 or above compiler

Build:

make 

Build tests:

make tests

Sample Usage:

./build/eventapp "fieldname1:fieldval1|fieldname2:fieldval2|..."

./build/eventapp "location:Vancouver|within:100|category:business|page_size:50"

Default Output:

"{ title: <title> | venue_name: <vanue name> | start_time: <start time> | venue_address: <venue address> }"
