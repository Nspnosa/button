
function navigateTo(page) {
    fetch(`${page}.html`)
        .then(response => response.text())
        .then(html => {
            document.getElementById('app').innerHTML = html;
        })
        .catch(error => console.error('Error fetching page:', error));
}

let configuration;
async function getConfiguration() {
    const response = await fetch("/configuration/button");
    configuration = await response.json();
    console.log(configuration);
}

let actions = [];
async function getActions() {
    for (let i = 1; i <= 5; i++) {
        const response = await fetch(`/configuration/actions/${i}`);
        actions.push(await response.json());
    }
    console.log(actions);
}

let credentials;
async function getCredentials() {
    const response = await fetch(`/configuration/credentials`);
    credentials = await response.json();
    console.log(credentials);
}

let accessPoints = [];
async function getAccessPoints() {
    const response = await fetch(`/configuration/ap`);
    accessPoints = await response.json();
    console.log(accessPoints);
}

let deviceConnected; //this should be a boolean instead of an object
async function getConnectionStatus() {
    const response = await fetch(`/configuration/connection`);
    deviceConnected = await response.json();
    console.log(deviceConnected);
}

let serverCredentials;
async function getServerCredentials() {
    const response = await fetch(`/configuration/servercredentials`);
    serverCredentials = await response.json();
    console.log(serverCredentials);
}

window.onload = async (event) => {
    await getConfiguration();
    await getActions();
    await getCredentials();
    await getServerCredentials();
}