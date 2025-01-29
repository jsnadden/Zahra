Planning:
- ~~give `Scene` a map taking an entity (by `GUID`) and returning a buffer for storing field values for an attached script, so that these can be set prior to `OnRuntimeStart`~~
- ~~(de)serialise this map's data in the `ScriptComponent` section for each such entity~~
- ~~in `OnRuntimeStart`, look for corresponding fields in the script and set their values from our map (doesn't matter if the lists of fields don't line up, just set the ones that need setting, and can be found in the map)~~
- ~~when a `ScriptComponent` is created with a valid `ScriptName`, allocate a corresponding buffer~~
- ~~when an extant `ScriptComponent` is edited, reallocate its storage buffer~~
- ~~the editor should be able to manipulate these buffer values in `Edit` mode, or the actual field values via the `ScriptInstance` in `Play` mode~~
- ~~when a `Scene` is duplicated, make an exact copy of the map's data, including the ACTUAL storage data, which the `Buffer` objects simply point to. Otherwise the new `Scene` will override the original's field values.~~
 
NOTES TO FUTURE SELF:
 - The engine is only able to read/write public fields with the custom `[ExposedField]` attribute
 - The `[EntityID]` attribute signals that a `ulong` should be interpreted as referring to an Entity, rather than just an abstract value (in reflection it is assigned `FieldType::Entity`)
 - To debug a C# script while `Zahra::ScriptEngine` is running, open the C# solution in VS, select **Debug > Attach Unity Debugger"** and input the (localhost) IP `127.0.0.1` at port `55555`.