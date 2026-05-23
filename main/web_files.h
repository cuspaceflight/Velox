#pragma once

static const char* main_page = R"RAWSTRING(
<!DOCTYPE html>
<html>
    <head>
        <title>Velox ground station</title>

        <script>
            function startup() {
                setInterval(function() {
                    getData();
                }, 500);
            }

            function getData() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        // console.log(this.responseText);
                        data = JSON.parse(this.responseText);
                        // console.log(data);

                        document.getElementById("TIME").innerHTML = data["TIME"]
                        document.getElementById("ENTR").innerHTML = data["ENTR"]
                        // document.getElementById("GLAT").innerHTML = data["GLAT"]
                        // document.getElementById("GLON").innerHTML = data["GLON"]
                        document.getElementById("GALT").innerHTML = data["GALT"]
                        document.getElementById("GSPD").innerHTML = data["GSPD"]
                        document.getElementById("BTEM").innerHTML = data["BTEM"]
                        document.getElementById("BPRE").innerHTML = data["BPRE"]
                        document.getElementById("BALT").innerHTML = data["BALT"]
                        document.getElementById("ITEM").innerHTML = data["ITEM"]
                        document.getElementById("IACX").innerHTML = data["IACX"]
                        document.getElementById("IACY").innerHTML = data["IACY"]
                        document.getElementById("IACZ").innerHTML = data["IACZ"]
                        document.getElementById("IGYX").innerHTML = data["IGYX"]
                        document.getElementById("IGYY").innerHTML = data["IGYY"]
                        document.getElementById("IGYZ").innerHTML = data["IGYZ"]

                        document.getElementById("GLAT_DEG").innerHTML = Math.floor(data["GLAT"] / 10000)
                        document.getElementById("GLAT_MIN").innerHTML = Math.floor((data["GLAT"] % 1000.0) / 100)
                        document.getElementById("GLAT_SEC").innerHTML = ((data["GLAT"]) % 100.0).toString().slice(0, 8)
                        document.getElementById("GLAT_DIR").innerHTML = data["GLAT"] > 0 ? "N" : "S"

                        document.getElementById("GLON_DEG").innerHTML = Math.floor(data["GLON"] / 10000)
                        document.getElementById("GLON_MIN").innerHTML = Math.floor((data["GLON"] % 1000.0) / 100)
                        document.getElementById("GLON_SEC").innerHTML = ((data["GLON"]) % 100.0).toString().slice(0, 8)
                        document.getElementById("GLON_DIR").innerHTML = data["GLON"] > 0 ? "N" : "S"
                    }
                }
                xhttp.open("GET", "data", true);
                xhttp.send();
            }
        </script>
    </head>
    <body onload="startup();">
        <div>
            <h1>Velox</h1>
            <h2>TIME: <span id="TIME"></span></h2>
            <h2>ENTRY: <span id="ENTR"></span></h2>
            <h1>GPS</h1>
            <h2>LAT: <span id="GLAT_DEG"></span>&deg; <span id="GLAT_MIN"></span>' <span id="GLAT_SEC"></span>'' <span id="GLAT_DIR"></span></h2>
            <h2>LON: <span id="GLON_DEG"></span>&deg; <span id="GLON_MIN"></span>' <span id="GLON_SEC"></span>'' <span id="GLON_DIR"></span></h2>
            <h2>ALT: <span id="GALT"></span></h2>
            <h2>SPD: <span id="GSPD"></span></h2>
            <h1>BMP</h1>
            <h2>TEMPERATURE: <span id="BTEM"></span></h2>
            <h2>PRESSURE   : <span id="BPRE"></span></h2>
            <h2>ALTITUDE   : <span id="BALT"></span></h2>
            <h1>MPU</h1>
            <h2>TEMP   : <span id="ITEM"></span></h2>
            <h2>ACCEL X: <span id="IACX"></span></h2>
            <h2>ACCEL Y: <span id="IACY"></span></h2>
            <h2>ACCEL Z: <span id="IACZ"></span></h2>
            <h2>GYRO  X: <span id="IGYX"></span></h2>
            <h2>GYRO  Y: <span id="IGYY"></span></h2>
            <h2>GYRO  Z: <span id="IGYZ"></span></h2>
        </div>
    </body>
</html>)RAWSTRING";
