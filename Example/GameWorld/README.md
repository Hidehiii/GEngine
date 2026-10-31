# GameWorld

This focused example exercises the scene world-logic tick, managed C# scripts,
statically registered native C++ scripts and the split physics backend modules.
It creates active, inactive and failing-script GameObjects, then dispatches
awake/start/enable, fixed and variable updates, late update, pause/resume,
disable/enable and deferred destruction. It also performs a Box2D step and a
PhysX init/shutdown smoke check.

The application uses the build-generated Mono BCL path and `GEngine-ScriptCore`
assembly path. Expected output includes `WorldLogicScript` lifecycle records,
one fixed update, variable updates, pause/resume, disable/enable and destroy,
plus one logged `FailingScript` managed exception and one logged native C++
script exception. The process then closes normally. No visible rendering result
is claimed.
