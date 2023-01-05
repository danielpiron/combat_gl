* [X] Move PNG/Texture loading into separate file.
  * [ ] More tightly integrate with the concept of resource loading
* [ ] Single color texture should also somehow be part of some Texture/Resource related file.
      Perhaps a little convention would help for single colors, like giving the color in HTML
      style. (e.g. "#139aCfe"). These could be potentially be used to generate textures as well
      and give them names. Ideally, if a texture hasn't already been registered, it should be
      loaded or generated.
      * [ ] Or just have a boolean on which to use texture and/or baseColor
* [X] "Entity" into its own file
* [X] Each entity should have its own .h file defining behavior
* [X] Move the render quad code into a Debug(.h/.cpp) file or something
* [X] Fix shadow mapping area 
* [X] Apply color to walls and floors
* [ ] Update Input query functions
  * [ ] if (applesauce:::Input::keyIsHeldDown(GLFW_KEY_SPACE)
  * [ ] if (applesauce::Input::keyWasJustPressed(GLFW_KEY_A))
* [ ] Encapsulate frame buffer
* [ ] Camera behavior should be defined by Entity logic
* [ ] Each tank needs to be bound to different controls.
* [ ] Figure out how to size shadow "frustum" dynamically
* [ ] Load levels from files
