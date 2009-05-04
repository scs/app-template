#! /bin/msh

# Kill all stray process instances.
killall app-template 2> /dev/null

# Copy the web interface to the http server's root directory.
echo "Setting up the web interface ..."
rm -rf /home/httpd/*
gzip -d < /mnt/app/www.tar.gz | tar -x -C /home/httpd/

# Run the application
echo "Running the application..."
/mnt/app/app-template_target
echo "The application quit with an exit status of $?."
