import logo from "./logo.svg";
import React, { useState } from "react";
import "./App.css";
import Box from "@mui/material/Box";
import Slider from "@mui/material/Slider";
import TextField from "@mui/material/TextField";
// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getDatabase, ref, set, update } from "firebase/database";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyAtY1d7kKLU69TpfWXP29pO5eX7cL5lguw",
  authDomain: "marvis-2090a.firebaseapp.com",
  databaseURL: "https://marvis-2090a-default-rtdb.firebaseio.com",
  projectId: "marvis-2090a",
  storageBucket: "marvis-2090a.appspot.com",
  messagingSenderId: "207201971772",
  appId: "1:207201971772:web:bf65df824083c4ecc25d39",
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
// Initialize Realtime Database and get a reference to the service
const database = getDatabase(app);

function App() {
  const [myText, setMyText] = useState("...");

  const sendBrightness = (event, newBrightness) => {
    const db = getDatabase();
    update(ref(db, "marvis"), {
      brightness: newBrightness,
    });
  };

  const sendFlashingSpeed = (event, newFlashingSpeed) => {
    const db = getDatabase();
    update(ref(db, "marvis"), {
      flashing_speed: newFlashingSpeed,
    });
  };

  const sendMotor = (event, newMotor) => {
    const db = getDatabase();
    update(ref(db, "marvis"), {
      motor: newMotor,
    });
  };

  const talk = (event) => {
    if (event.key === "Enter") {
      const requestOptions = {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Authorization: "Bearer " + String(""),
        },
        body: JSON.stringify({
          prompt: event.target.value,
          temperature: 0.4,
          max_tokens: 200,
          top_p: 1,
          frequency_penalty: 0,
          presence_penalty: 0.5,
          stop: ['"""'],
        }),
      };
      fetch(
        "https://api.openai.com/v1/engines/text-davinci-003/completions",
        requestOptions
      )
        .then((response) => response.json())
        .then((data) => {
          console.log(data.choices[0].text);
          setMyText(data.choices[0].text);
        })
        .catch((err) => {
          console.log("Ran out of tokens for today! Try tomorrow!");
        });
    }
  };

  return (
    <div className="App">
      <header className="App-header">
        <p>Greetings, I am Marvis.</p>
        <div className="Slider">
          <h4 className="Header">Brightness</h4>
          <Box sx={{ width: 250 }}>
            <Slider
              aria-label="Custom marks"
              defaultValue={50}
              max={255}
              onChangeCommitted={sendBrightness}
              valueLabelDisplay="on"
            />
          </Box>
        </div>
        <div className="Slider">
          <h4 className="Header">Flashing Speed</h4>
          <Box sx={{ width: 250 }}>
            <Slider
              aria-label="Custom marks"
              defaultValue={50}
              max={255}
              onChangeCommitted={sendFlashingSpeed}
              valueLabelDisplay="on"
            />
          </Box>
        </div>
        <div className="Slider">
          <h4 className="Header">Motor</h4>
          <Box sx={{ width: 250 }}>
            <Slider
              aria-label="Custom marks"
              defaultValue={0}
              min={-10}
              max={10}
              marks={true}
              color="secondary"
              onChangeCommitted={sendMotor}
              valueLabelDisplay="on"
            />
          </Box>
        </div>
        <br></br>
        <TextField
          sx={{ input: { color: "white" } }}
          id="fullWidth"
          variant="filled"
          label="Talk to me!"
          size="large"
          focused
          style={{ width: "300px" }}
          onKeyPress={talk}
        />
        <div className="Response">
          <p>{myText}</p>
        </div>
      </header>
    </div>
  );
}

export default App;
