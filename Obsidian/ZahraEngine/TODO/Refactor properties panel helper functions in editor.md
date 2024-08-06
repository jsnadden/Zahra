Refactor the UI functions in the following manner:
```
DrawControls<ComponentType> (reference to entity)
{
StartComponentControls(some config args)
for (each member var): DrawControl<var type>(var)
EndComponentControls()
}

StartComponentControls()
//includes PushID based on entity guid and component type

EndComponentControls()

DrawControl<var type>(reference to var)
//variable name, new column, widget, new column
```
If we have [[ECS Reflection & Serialisation|component reflection]] in place, the templates won't need explicit specialisations