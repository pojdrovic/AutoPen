import sqlite3 as lite
import os


##############################################
#       FIRST CHECK TO SE IF DB EXISTS
#############################################

# Database Names
generalDBName = "autoPenGeneralData.sqlite"
testDBName = "testDB.sqlite"


# Temporary variables to store existance of databse.
generalDBExists = not os.path.exists(generalDBName)
testDBExists = not os.path.exists(testDBName)



#########
# INIT #
########

def init():
        # First check to see if database already exists.
        print("nothing")



########################################################################
# Function definitions to build databases if they do not already exists#
########################################################################

# Method to build the general purpose database
def generalDBBuild():
        dbConn = lite.connect(generalDBName)
        c = dbConn.cursor()

def testDBBuild():
        testDbConn = lite.connect(testDBName)
        testCuror = testDbConn.cursor()



############################################################
# Function to create DB for a car and function to add to it#
############################################################

def createDBforCar(dbName, tableName):
        print("Nothing~")

def addToCarDB(dbName, tableName, timestsamp, canField):
        print("Nothing")


###################################
# Function to add user to program #
###################################

def addUser(firstName, lastName, userName):
        print("nothing")







# GARBAGE - DELETE LATER!
#############################################

sqlite_filename = "autoPenData.sqlite"

db_is_new = not os.path.exists(sqlite_filename)

print("Inside the dbutils helper method")




sqlite_file = 'my_first_db.sqlite'    # name of the sqlite database file
table_name1 = 'my_table_1'  # name of the table to be created
table_name2 = 'my_table_2'  # name of the table to be created
new_field = 'my_1st_column' # name of the column
field_type = 'INTEGER'  # column data type

db_is_new1 = not os.path.exists(sqlite_file)

if db_is_new1:
        conn = lite.connect(sqlite_file)
        c = conn.cursor()

        c.execute('CREATE TABLE {tn} ({nf} {ft})'\
                .format(tn=table_name1, nf=new_field, ft=field_type))

        # Creating a second table with 1 column and set it as PRIMARY KEY
        # note that PRIMARY KEY column must consist of unique values!
        c.execute('CREATE TABLE {tn} ({nf} {ft} PRIMARY KEY)'\
                .format(tn=table_name2, nf=new_field, ft=field_type))

        conn.commit()
        conn.close()



# Connecting to the database file
conn = lite.connect(sqlite_file)
c = conn.cursor()


# Committing changes and closing the connection to the database file
conn.commit()
conn.close()