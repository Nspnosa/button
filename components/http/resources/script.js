async function getConfiguration() {
    const response = await fetch("/configuration/button");
    const configuration = await response.json();
    return configuration;
}

async function getServerCredentials() {
    const response = await fetch(`/configuration/servercredentials`);
    const serverCredentials = await response.json();
    return serverCredentials;
}

async function getAccessPoints() {
    const response = await fetch(`/configuration/ap`);
    const accessPoints = await response.json();
    return accessPoints;
}

async function getCredentials() {
    const response = await fetch(`/configuration/credentials`);
    const credentials = await response.json();
    return credentials;
}

async function getAction(id) {
    const response = await fetch(`/configuration/actions/${id}`);
    const action = await response.json();
    return action;
}

//this should be a boolean instead of an object
async function getConnectionStatus() {
    const response = await fetch(`/configuration/connection`);
    const deviceConnected = await response.json();
    return deviceConnected;
}

let currentPath = null;

async function navigateTo(page) {
    let pageList = page.split('-');
    let jsonData = {};

    document.getElementById('data').textContent = "LOADING...";

    deleteButton.disabled = false;
    updateButton.disabled = false;

    currentPath = pageList;
    switch(pageList[0]) {
        case "configure":
            jsonData = await getConfiguration();
            break;
        case "provision":
            jsonData = await getCredentials();
            break;
        case "accessPoints":
            for (let button of buttons) {
                button.disabled = true;
            }
            jsonData = await getAccessPoints();
            break;
        case "security":
            jsonData = await getServerCredentials();
            break;
        case "action":
            jsonData = await getAction(pageList[1]);
            break;
    }
    let prettyPrintedData = JSON.stringify(jsonData, null, 2);
    document.getElementById('data').textContent = prettyPrintedData;
}

async function navigationEventListener (event) {
    const elements = document.getElementsByTagName("a");
    for (let element of elements) {
        element.className = "navigation";
    }
    event.target.className = "navigation-selected"
    await navigateTo(event.target.id);
}

let deleteButton;
let updateButton;

window.onload = async (event) => {
    const elements = document.getElementsByClassName("navigation");
    for (let element of elements) {
        element.addEventListener("click", navigationEventListener);
    }

    deleteButton = document.getElementById('delete-button');
    updateButton = document.getElementById('update-button');

    deleteButton.disabled = true;
    updateButton.disabled = true;
    updateButton.addEventListener("click", updateButtonAction);
}

async function updateButtonAction(event) {
    let path = "";
    switch(currentPath[0]) {
        case "configure":
            path = "/configuration/button";
            break;
        case "provision":
            path = "/configuration/credentials";
            break;
        case "security":
            path = "/configuration/servercredentials";
            break;
        case "action":
            path = `/configuration/actions/${currentPath[1]}`;
            break;
    }
    try {
        const response = await fetch(path, {
            method: "POST",
            body: document.getElementById('data').textContent, 

        });
    } catch (e) {
        console.log(e);
    }
    const prettyPrintedData = JSON.stringify(response, null, 2);
    document.getElementById('data').textContent = prettyPrintedData;
}

function deleteButtonAction(event) {
    switch(currentPath) {
        case "configure":
            break;
        case "provision":
            break;
        case "security":
            break;
        case "action":
            break;
    }
}