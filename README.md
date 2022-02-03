<h1>Castle - scene</h1>
<p>The subject of the project consists in the photorealistic presentation of 3D objects using
OpenGL library. The user directly manipulates by mouse and keyboard inputs the scene of
objects. OpenGL (Open Graphics Library) is a cross-language, cross-platform application
programming interface (API) for rendering 2D and 3D vector graphics. The API is typically
used to interact with a graphics processing unit (GPU), to achieve hardware-accelerated
rendering. I chose to implement a scene where you can go and visit an old castle in the
skies, move an old military car around the scene and watch how wind blows in some of the
tree’s leaves. You can choose to move the camera in any wanted direction so you can
discover every element from this scene.</p>

</br>

<img src="https://user-images.githubusercontent.com/24292234/152395449-217efd2a-7b95-45a3-80be-c7f586e5ff4c.png"/>

</br>

<p> 
Functionalities implemented are visualization of the scene using a camera, multiple view
frames of the same scene, wind and fog effect, multiple sources of light, animated and
moving objects.
</p>
<dl><b>Visualization</b> - When we're talking about camera/view space we're talking about all
the vertex coordinates as seen from the camera's perspective as the origin of the
scene: the view matrix transforms all the world coordinates into view coordinates that
are relative to the camera's position and direction.
    </dl>
    
 <dl><b>Multiple view frames</b> - Representation of the scene switching between Solid View
Wireframe View and Polygonal view using keyboard keys.
    </dl>

 <dl><b> Wind and fog effect</b> - We can increase the realism of a 3D scene by adding fog or
wind effect which will create a particular atmosphere. By manipulating the attributes
associated with the fog effect we can personalize the atmosphere and also we
enhance the perception of depth.</dl>
    
 
