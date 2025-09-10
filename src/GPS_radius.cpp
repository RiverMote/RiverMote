#include <Arduino.h>
#include <math.h>

double degreesToRadians(double degrees) {
  return degrees * PI / 180.00;
}

//-- a function to get the distance from two lat,lon positions --//
double getDist(double lat1, double lon1, double lat2, double lon2)
{
    double earthRadiusKm = 6371.1370;

    double dLat = degreesToRadians(lat2-lat1);
    double dLon = degreesToRadians(lon2-lon1);

    lat1 = degreesToRadians(lat1);
    lat2 = degreesToRadians(lat2);
    
    double a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
    double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
    double dist = earthRadiusKm * c;    
    return dist; // in kilometers
}


// function to trim last digits
// test # 41.906334290146226 / 41.906334
void trim_gps(double coord) {
    double whole_num = long(coord); 
    double remainder = (coord - whole_num);
    delay(1000); Serial.print(coord); 
}

// From the web
/*
0 decimal places: ~111 km (very rough location)
1 decimal place: ~11.1 km
2 decimal places: ~1.11 km
3 decimal places: ~111 meters
4 decimal places: ~11.1 meters
5 decimal places: ~1.11 meters
6 decimal places: ~0.111 meters (11.1 cm)

/*
//-- List of locations to track --//
String target_name[]  = {"Home"        , "School"  ,"Park"        ,"Dad"    ,"Mcd"    ,"Foot Locker","Sister- Maywood"  ,"Unknown"     , "Albert Home"};    // places by name
double target_lat[]   = { 41.886017    , 41.869355 , 41.8821    , 41.978857, 41.866720, 41.896382   , 41.886401        , 40.761497, 41.880544    }; 
double target_lon[]   = {-87.710663    ,-87.708744 ,-87.7169    ,-87.657480,-87.706163,-87.707260   ,-87.833976        ,-73.958786,-87.625706    };
double target_radius[]= {0.004         ,0.025      ,0.05        ,0.004     ,0.004     ,0.009         ,5                ,10.1      ,0.04          }; // in kM
int target_rating[]   = {1             ,2          ,3            ,1        ,3         ,1            ,1                 ,8        ,7             };

double curr_lat, curr_lon = 0.000000;
int curr_loc, last_loc = -1;

// This fuction loops through all the locations //
double checkDist()
{
    for(int i=0; i < arraySize(target_lat); i++ ) // Note: we use lat for array size because all arrays should be the same length
    {
        double t_lat = target_lat[i];
        double t_lon = target_lon[i];
        String name  = target_name[i];
        double t_rad = target_radius[i];
        int t_rating = target_rating[i];
        
        double dist = getDist(curr_lat, curr_lon, t_lat, t_lon); //Serial.println(dist); delay(500);
        
        // if in radius, broadcast
        if(dist < t_rad) // if you are within the radius 
        {
            curr_loc = i;
            
            broadcast(i, name, t_rating);
            //--TEST--//
            #ifdef DEBUG
                //sprintf("Location %d: ", curr_loc);
                //Serial.print(curr_loc);Serial.print(":");Serial.print(name);Serial.print(" ");Serial.println(t_rad); delay(500);
            #endif
        } //else {
            //int t_rating = 9;
        }
        
    }
//}

//-- a function to get the distance from two lat,lon positions --//
double getDist(double lat1, double lon1, double lat2, double lon2)
{
    double earthRadiusKm = 6371.1370;

    double dLat = degreesToRadians(lat2-lat1);
    double dLon = degreesToRadians(lon2-lon1);

    lat1 = degreesToRadians(lat1);
    lat2 = degreesToRadians(lat2);
    
    double a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
    double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
    double dist = earthRadiusKm * c;    
    return dist; // in kilometers
}


double degreesToRadians(double degrees) {
  return degrees * PI / 180;
}

// An example I found on StackOverflow (thank you!) //
/*
function degreesToRadians(degrees) {
  return degrees * Math.PI / 180;
}

function distanceInKmBetweenEarthCoordinates(lat1, lon1, lat2, lon2) {
  var earthRadiusKm = 6371;

  var dLat = degreesToRadians(lat2-lat1);
  var dLon = degreesToRadians(lon2-lon1);

  lat1 = degreesToRadians(lat1);
  lat2 = degreesToRadians(lat2);

  var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
          Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2); 
  var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a)); 
  return earthRadiusKm * c;
}
*/