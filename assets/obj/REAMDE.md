# Processing

The AutoDesk Forge API is used to convert the BIM model into an `.obj` file, a plain text file containing definition of vertices and faces (BRep). 

> SHORTCOMINGS   
> The geometry is exported without the attributes. Attributes are exported using another endpoint as a `.json` file. However, for the C3D file, the *name* attribute is not exported like expected. In the `properties.db` which can also exported, the name is available, however, the format of the `.db` is not as nice as the `.json` file. I'm in contact with AutoDesk to fix this.

## `.obj` file and `.json` file

Those files are generated using the AutoDesk Forge API. 

### Translation of the `BIM Model.obj`

A translation is required to move the `BIM Model.obj` geometry to the same coordinate system as the `Hybride grondmodel.obj`. 

X-translation = + 153700 (m)  
Y-translation = + 214700 (m)

