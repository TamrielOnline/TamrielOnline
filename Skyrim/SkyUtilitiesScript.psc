Scriptname SkyUtilitiesScript extends Form

int function GetMessageCount() global native

int function ReadMessageGuid() global native

string function ReadMessageName() global native

Form function ReadMessageRef() global native

string[] function ReadMessageValues() global native

function MountActor(Actor ref, ObjectReference mountRef) global native

function ResurrectExtended(Actor ref) global native

function SetRace(Actor ref, string raceName) global native

function APS(Actor ref, string command) global native

function SetDisplayName(Actor ref, string dName) global native

function StartTimer(string tName) global native

bool function GetRemotePlayerDataBool(int refId, string tName) global native

float function GetRemotePlayerDataFloat(int refId, string tName) global native

int function GetRemotePlayerDataInt(int refId, string tName) global native

string function GetRemotePlayerDataString(int refId, string tName) global native

function SetRemotePlayerDataBool(int refId, string tName, bool val) global native

function SetRemotePlayerDataFloat(int refId, string tName, float val) global native

function SetRemotePlayerDataInt(int refId, string tName, int val) global native

function SetRemotePlayerDataString(int refId, string tName, string val) global native

bool function HasSecondsPassed(string tName, float seconds) global native

bool function HasMillisecondsPassed(string tName, float milliseconds) global native

function InitializeNewPlayer(int ref, int movementController, int targetController, int guid) global native

string function GetSitAnimation(Actor ref) global native

function EquipItem(Actor ref, Form item, int slot) global native

function SetTransform(string ref, int x, int y, int z, int xRot, int yRot, int zRot) global native

Form function Retrieve(string ref) global native

Actor function RetrieveActor(string ref) global native

ObjectReference function RetrieveObject(string ref) global native

function EnableNet(Actor ref) global native

function DisableNet(Actor ref) global native

function PlayerKOFA(Actor ref, int id) global native