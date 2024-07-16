# Use the official Python base image
FROM python:3.9-slim

# Set the working directory in the container
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . /app

# Install any needed packages specified in requirements.txt
# (For this example, we assume no external Python dependencies)
# If you have dependencies, uncomment the following line and create a requirements.txt file
# RUN pip install --no-cache-dir -r requirements.txt

# Expose the port the server will run on
EXPOSE 9001

# Define environment variable
ENV SOCK_IP=0.0.0.0
ENV SOCK_PORT=9001

# Run app.py when the container launches
CMD ["python", "app.py"]

