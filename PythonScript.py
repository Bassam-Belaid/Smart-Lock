import mysql.connector
import serial
import time
import cv2
import os

class User:

    def __init__(self, name, card_code):
        self.name = name
        self.card_code = card_code

    def __str__(self):
        return f"Name: {self.name}, Card Code: {self.card_code}"

    @staticmethod
    def find(card_code):
        # Connect to the MySQL database
        connection = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            database="lock_system"
        )

        if connection.is_connected():
            print("Connected to database")
        else:
            print("Error connecting to database")
            return None

        # Get a cursor object
        cursor = connection.cursor()

        # Execute a SQL query to find the user with the given card_code
        query = "SELECT user_name, user_card FROM users WHERE user_card = %s"
        cursor.execute(query, (card_code,))

        # Fetch the result
        result = cursor.fetchone()

        # Close the cursor and connection
        cursor.close()
        connection.close()

        if result:
            # Create a User object and return it
            return User(result[0], result[1])
        else:
            # No user found, return None
            return None

    @staticmethod
    def add_user(name, card_code):
        # Connect to the MySQL database
        connection = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            database="lock_system"
        )

        if connection.is_connected():
            print("Connected to database")
        else:
            print("Error connecting to database")
            return False

        # Get a cursor object
        cursor = connection.cursor()

        # Execute a SQL query to insert the new user
        query = "INSERT INTO users (user_name, user_card) VALUES (%s, %s)"
        cursor.execute(query, (name, card_code))
        connection.commit()

        # Close the cursor and connection
        cursor.close()
        connection.close()

        return True

s = serial.Serial(port='COM5', baudrate=9600)

current_user = None
new_user = None
user_name = "";

if not os.path.exists("Persons"):
    os.makedirs("Persons")

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    try:
        if s.in_waiting > 0:
            value = s.readline()
            valueInString = str(value, 'UTF-8').strip()
            print(valueInString)

            if valueInString != "":

              current_user = User.find(valueInString)

              if current_user != None:
                s.write(str(current_user.name).encode())
                s.flush()
                print("Done")

            if valueInString.lower() == 'add new':
                s.flush()

                if s.in_waiting > 0:
                    value2 = s.readline()
                    valueInString2 = str(value2, 'UTF-8').strip()
                    print(valueInString2)

                    user_name = input("Enter your name: ")
                    s.write("done".encode())
                    s.flush()

                    if User.add_user(user_name, valueInString2):
                       print("New user added successfully")
                       s.flush()

                    else:
                       print("Failed to add new user")

            elif valueInString.lower() == 'take image':

                ret, frame = cap.read()
                image_path = os.path.join("Persons", f"captured_image_{len(os.listdir('Persons'))}.jpg")
                cv2.imwrite(image_path, frame)
                print(f"Image saved as '{image_path}'")

    except UnicodeDecodeError:
        try:
            valueInString = str(value, 'cp-1252').strip()
            print(valueInString)
            if valueInString.lower() == 'yes':
                ret, frame = cap.read()
                image_path = os.path.join("Persons", f"captured_image_{len(os.listdir('Persons'))}.jpg")
                cv2.imwrite(image_path, frame)
                print(f"Image saved as '{image_path}'")
        except UnicodeDecodeError:
            valueInString = str(value, 'latin-1').strip()
            print(valueInString)
            if valueInString.lower() == 'yes':
                ret, frame = cap.read()
                image_path = os.path.join("Persons", f"captured_image_{len(os.listdir('Persons'))}.jpg")
                cv2.imwrite(image_path, frame)
                print(f"Image saved as '{image_path}'")
    except serial.SerialTimeoutException:
        print("No data received. Waiting for 1 second.")
        time.sleep(1)

cap.release()
cv2.destroyAllWindows()
