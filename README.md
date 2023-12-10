## 12/9/2023

Finally did a refactor, trying to split out the the platform specific stuff from the actual game logic. Started by sticking the internal loop into it's own function and file. Added the include and progressively moved the include up. Once I hit a compiler error, I moved the appropriate functions into the breakout.cpp, generally this meant that it was game specific. Except DrawQuad, that was annoying because it ended up being the only function left with openGL function calls.

To avoid this, I decided that the game would define a RenderQuad data struct, then the platform layer would provide a `std::vector<QuadRenderData>` that the game could push to. Then the platform layer would call DrawQuad.

The last thing I did was add some more behavior on ball collision. I think that I probably need to do the strategy pattern since I want different behavior depending on what the ball is colliding with.

Another annoying thing is that the game is all done in pixels, so I need to know the window width / height to render.

I think next I also want to do some audio stuff.
