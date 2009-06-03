#! /bin/msh

# Go into the directory of the application.
P=`readlink -f "$0"`
cd "`dirname "$P"`" || exit $?

# Kill all stray process instances.
echo "Killing other instances of the application ..."
killall app 2> /dev/null

# Copy the web interface to the http server's root directory.
echo "Setting up the web interface ..."
rm -rf /home/httpd/*
gzip -d < www.tar.gz | tar -x -C /home/httpd/

# Run the application
echo "Running the application..."
./app
echo "The application quit with an exit status of $?."
