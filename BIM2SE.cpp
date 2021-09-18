#include <iostream>
#include <BIM2SE.h>

// OCCT library
// - Basic data structures (geometry)
// - Modeling algorithms
// - Working with mesh (faceted) data
// - Data interoperability

// Grouped in packages > toolkits (libraries; .so or .dll) > modules

// Data types:
// 1. Primitive types (Boolean, Character, Integer, Real,..) --> Manipulated by value
// 2. OCCT classes --> Manupulated by handle (= on reference to an instance)
// Handle = safe way to manipulate object

// Standard package
#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx> // Primitive type of the Standard package
#include <Standard_Transient.hxx> // Primitive type of the Standard package

// Retrieve type descriptor -> STANDARD_TYPE(Geom_Line) for example
#include <gp_Pnt.hxx> // Point - short lived
#include <gp_Trsf.hxx> // Transformation
#include <NCollection_Array2.hxx> // To define 2D Arrays
#include <TColgp_Array2OfPnt.hxx> 

// Geom package - part of TKG3d toolkit (.dll - need to be linked in CMakeLists)
#include <Geom_Point.hxx> // Type manipulated by handle - Abstract class
#include <Geom_CartesianPoint.hxx>  // Concrete class
#include <Geom_BSplineSurface.hxx> 

// GeomAPI package - part of TKGeomAlgo toolkit (.dll - need to be linked in CMakeLists)
#include <GeomAPI_PointsToBSplineSurface.hxx> // Surface approximation

// BrepPrimAPI package - part of TKPrim toolkit
#include <BRepPrimAPI_MakeBox.hxx> // Make a Box
#include <BRepPrimAPI_MakeCylinder.hxx> // Make a Cylinder
// BRepPrimAPI_MakeHalfSpace

// BRepMesh package - part of TKMesh toolkit
#include <BRepMesh_IncrementalMesh.hxx> // Make a mesh from a topological data structure
#include <BRepMesh_ModelPostProcessor.hxx> // Make TopoDS_Edge from mesh

// BRepBuilderAPI package - part of the TKTopAlgo toolit
#include <BRepBuilderAPI_MakeFace.hxx> // Build a face from a surface
#include <BRepBuilderAPI_MakePolygon.hxx> // Build a polygon
#include <BRepBuilderAPI_Transform.hxx> // Transformation

// BRepAlgoAPI package - part of TKBO toolkit
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>

// ShapeBuild_ReShape - part of the TKShHealing toolkit
#include <ShapeBuild_ReShape.hxx>

// Writa a STL file - tessellated format (triangulation) - part of TKSTL toolkit
#include <StlAPI_Writer.hxx>

// Read a STL file - tessellated format (triangulation) - part of TKSTL toolkit
#include <RWStl.hxx>

// Poly package - part of the TKMath toolkit
#include <Poly_Triangulation.hxx> // Provide triangulation for a surface
#include <Poly_Triangle.hxx> // Describes a component triangle of a triangulation

// Write a STEPfile
#include <STEPControl_Writer.hxx>

// Include GProp package - calculate global properties
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

// TopoDS package - part of the TKBRep toolkit
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>

// Type definitions
typedef NCollection_Array2<gp_Pnt> ListOfPoints; // the same as TColgp_Array2OfPnt
// https://3dinsider.com/stl-vs-obj/

// Include goodbye if set
#ifdef USE_SANDBOX
#   include <sandbox.h>
#endif

/*
  Split a shape into multiple subshapes
*/
// Inspiration https://github.com/bigOrange123456789/PYTHON/blob/ede1c2eb83309594577aa565e14c3b67e3e97ef4/ifc/IfcOpenShell-0.6.0/src/ifcgeom/IfcGeomFunctions.cpp
TopTools_ListOfShape BIM2SE_SubShape(const TopoDS_Shape& in) {
    TopTools_ListOfShape out;
		TopoDS_Iterator shapeIterator(in);
		for (; shapeIterator.More(); shapeIterator.Next()) {
			out.Append(shapeIterator.Value());
		}
    return out;
}

/*
  Read an STL file and convert it to a BRep
*/
// Inspiration https://github.com/ncPUMA/FanucBotGui/blob/951252685621165e97e2f0739a7c73a1e78b77cd/src/ModelLoader/cstlloader.cpp
TopoDS_Shape BIM2SE_ReadSTL(const char *filename)
{
  TopoDS_Shape BRepModel;
  Handle(Poly_Triangulation) STLModel = RWStl::ReadFile(filename);

  if (not STLModel.IsNull())
  {
    BRep_Builder shellBuilder;
    TopoDS_Shell shell;
    shellBuilder.MakeShell(shell);
    for (const Poly_Triangle triangle : STLModel->Triangles())
    {
      Standard_Integer index0, index1, index2;
      // Populate coords in the variables
      triangle.Get(index0, index1, index2);
      const gp_Pnt pnt0 = STLModel->Node(index0);
      const gp_Pnt pnt1 = STLModel->Node(index1);
      const gp_Pnt pnt2 = STLModel->Node(index2);
      // Now make a wire and a face for in the BRep model
      const TopoDS_Wire wire = BRepBuilderAPI_MakePolygon(pnt0, pnt1, pnt2, Standard_True);
      const TopoDS_Face face = BRepBuilderAPI_MakeFace(wire, Standard_True);
      // add the face to the shell
      shellBuilder.Add(shell, face);
    }
    BRepModel = shell;
  } else {
    std::cout << "the file '" << filename << "' couldn't be found" << std::endl;
  }

  return BRepModel;
}

/*
  Write an STEP file
*/
void BIM2SE_WriteSTEP(const TopoDS_Shape& shape, const char *filename)
{
  // Start the writer
  STEPControl_Writer STEPwriter;
  // Write the STL file
  STEPwriter.Transfer(shape, STEPControl_AsIs);
  STEPwriter.Write(filename);
}

/*
  Write an STL file
*/
void BIM2SE_WriteSTL(const TopoDS_Shape& shape, const char *filename)
{
  // Start the writer
  StlAPI_Writer STLwriter;
  // Mesh the object
  BRepMesh_IncrementalMesh meshedShape (shape, 1e-2, Standard_True);
  TopoDS_Shape meshedShapeExport = meshedShape.Shape();
  // Write the STL file
  STLwriter.Write(meshedShapeExport, filename);
}

int main(int argc, char *argv[])
{
  // Introduction
  std::cout << "BIM2SE - (c) " << AUTHOR << std::endl;

  /*
    In[1]: Define a BOX geometry AND cut away a volume
  */

  // Create a simple box
  gp_Pnt lowerLeftCornerOfBox (-50.0, -50.0, 0.0);
  BRepPrimAPI_MakeBox boxMaker (lowerLeftCornerOfBox, 100, 100, 100);
  TopoDS_Shape box = boxMaker.Shape();

  // Create a cylinder
  BRepPrimAPI_MakeCylinder cylinderMaker(25.0, 50.0);
  TopoDS_Shape cylinder = cylinderMaker.Shape();

  // Cut the cylinder out of the box
  BRepAlgoAPI_Cut cutMaker(box, cylinder);
  TopoDS_Shape boxWithHole = cutMaker.Shape();

  // Make the export writers ready
  STEPControl_Writer STEPwriter;
  StlAPI_Writer STLwriter;

  /*
    In[1]: Read the original geometry and translate the BIM model
  */ 

  // The result are very big models. Might be interesting to 
  // reduce the amount of meshed by looking for coplaner neirgbouring points
  // and deleting them. 
  TopoDS_Shape BIMmodel = BIM2SE_ReadSTL("assets/obj/BIM model.stl");
  TopoDS_Shape grondModel = BIM2SE_ReadSTL("assets/obj/hybride grondmodel.stl");

  if((not BIMmodel.IsNull()) and (not grondModel.IsNull())){
    std::cout << "Files is loaded" << std::endl;
    // Translate the BIMmodel
    gp_Trsf translateBIMmodel;
    translateBIMmodel.SetTranslation(gp_Pnt(0,0,0), gp_Pnt(153700, 214700,0));

    // Execute the translation
    BRepBuilderAPI_Transform BIMmodelTranslated(BIMmodel, translateBIMmodel);
    TopoDS_Shape BIMmodelFixed = BIMmodelTranslated.Shape(); 

    // Combine the models and write to a file
    TopoDS_Compound combined;
    TopoDS_Builder aBuilder;
    aBuilder.MakeCompound(combined);
    aBuilder.Add(combined, grondModel);
    aBuilder.Add(combined, BIMmodelFixed);

    // Write the compound file
    // --> The step file is 3GB in size!
    // --> the STL file contains strongly deformed geometry, but is smaller in size
    BIM2SE_WriteSTEP(combined, "combined.stp"); // ca. 360 mb in size!
    BIM2SE_WriteSTL(combined, "combined.stl"); // ca. 3 Gb in size!
  }

  /*
    In[2]: Define a surface using 4 points - approximate 
  */

  // Define 4 point which will be used to approximate a surface
  gp_Pnt aPnt1 (79, 87, 26);
  gp_Pnt aPnt2 (-62, 93, 84);
  gp_Pnt aPnt3 (-97, -61, 3);
  gp_Pnt aPnt4 (65, -65, 65);

  // Construct a surface - Generate 2Array of Points
  // Piont(i, j) where i is used for U, j for V
  TColgp_Array2OfPnt aPointList (1, 2, 1, 2);
  aPointList.SetValue(1, 1, aPnt1);
  aPointList.SetValue(1, 2, aPnt2);
  aPointList.SetValue(2, 2, aPnt3);
  aPointList.SetValue(2, 1, aPnt4);

  // Approximate the surface - interpolation would also be an option (same API)
  GeomAPI_PointsToBSplineSurface aPointsToBSplineSurface;
  #ifdef USE_SANDBOX
    aPointsToBSplineSurface = (aPointList);  
  #endif

  // Check if the surface has been created
  if(aPointsToBSplineSurface.IsDone())
  {
    // If the surface has been created, print a message
    std::cout << "Creation of Surface succeeded!" << std::endl;
    // Return the surface - downcasting
    Handle(Geom_BSplineSurface) aBSplineSurface = aPointsToBSplineSurface.Surface();
    // Handle(Geom_Surface) aSurface = Handle(Geom_Surface)::DownCast(aBSplineSurface);
    // Convert the surface to a face - second param the tolerance
    TopoDS_Face soilSurface = BRepBuilderAPI_MakeFace(aBSplineSurface, 1e-6);

    /* 
      In[3]: Make the intersection between the surface and the box
    */

    // Make a sectionMaker and execute immediatly
    BRepAlgoAPI_Section sectionMaker (soilSurface, boxWithHole, Standard_False);
    sectionMaker.Approximation(Standard_True);
    sectionMaker.Build();
    
    if(sectionMaker.IsDone())
    {
      std::cout << "Geometry has been sectioned (intersection of box and surface)" << std::endl;

      // Retrieve the geometry
      TopoDS_Shape shape1 = sectionMaker.Shape();
      TopoDS_Shape shape2 = sectionMaker.Shape1();
      TopoDS_Shape shape3 = sectionMaker.Shape2();
      TopTools_ListOfShape aListOfShape = sectionMaker.Tools();

      STEPControl_Writer STEPwriter;
      STEPwriter.Transfer(shape1, STEPControl_AsIs);
      STEPwriter.Write("shape1.stp");

      // Shape 1 - Export
      BRepMesh_IncrementalMesh meshedShape1 (shape1, 1e-2, Standard_True);
      TopoDS_Shape Shape1Export = meshedShape1.Shape();
      STLwriter.Write(Shape1Export, "shape1.stl");

      // Shape 2 - Export
      BRepMesh_IncrementalMesh meshedShape2 (shape2, 1e-2, Standard_True);
      TopoDS_Shape Shape2Export = meshedShape2.Shape();
      STLwriter.Write(Shape2Export, "shape2.stl");

      // Shape 3 - Export
      BRepMesh_IncrementalMesh meshedShape3 (shape3, 1e-2, Standard_True);
      TopoDS_Shape Shape3Export = meshedShape3.Shape();
      STLwriter.Write(Shape3Export, "shape3.stl");
    }

    /* 
      In[4]: Split the volume with a TopTools_ListOfShape
    */

    // Make the split, geometry should be grouped inside Topo
    TopTools_ListOfShape Objects; // Shapes that will be split
    TopTools_ListOfShape Tools; // Shapes by which the Objects will be split

    Objects.Append(boxWithHole);
    Tools.Append(soilSurface);

    BRepAlgoAPI_Splitter splitter;
    splitter.SetArguments(Objects);
    splitter.SetTools(Tools);
    // Avoid original shapes to be modified
    splitter.SetNonDestructive(Standard_True);
    splitter.SetUseOBB(Standard_True);
    splitter.Build();

    if(splitter.HasErrors())
    {
      // Print the error if they occur
      splitter.DumpErrors(std::cout);
    }

    if(splitter.IsDone())
    {
      // see also https://github.com/bigOrange123456789/PYTHON/blob/ede1c2eb83309594577aa565e14c3b67e3e97ef4/ifc/IfcOpenShell-0.6.0/src/ifcgeom/IfcGeomFunctions.cpp
      std::cout << "Slicer activated... (see FreeCAD Part Slice)" << std::endl;

      TopoDS_Shape test123 = splitter.Shape();
      
      // Write a STEP file
      STEPControl_Writer STEPwriter;
      STEPwriter.Transfer(test123, STEPControl_AsIs);
      STEPwriter.Write("test123.stp");

      // Write a STL file
      BRepMesh_IncrementalMesh meshedTest123 (test123, 1e-2, Standard_True);
      TopoDS_Shape test123Export = meshedTest123.Shape();
      STLwriter.Write(test123Export, "test123.stl");

      // Split the geometry in individual parts
      TopTools_ListOfShape subShape = BIM2SE_SubShape(test123);

      int i = 1;
      std::string filename;
      GProp_GProps volumeProp;
      for(TopTools_ListIteratorOfListOfShape it(subShape); it.More(); it.Next(), ++i)
      {
        // Write the face (soilSurface) to a file
        filename = "slice" + std::to_string(i);
        // Write a STEP file
        STEPControl_Writer STEPwriter;
        STEPwriter.Transfer(it.Value(), STEPControl_AsIs);
        STEPwriter.Write((filename + ".stp").c_str());
        // Write a STL file
        BRepMesh_IncrementalMesh geom (it.Value(), 1e-2, Standard_True);
        TopoDS_Shape geomExport = geom.Shape();
        STLwriter.Write(geomExport, (filename + ".stl").c_str() );

        // Calculate the volume
        BRepGProp::VolumeProperties(it.Value(), volumeProp);
        std::cout << std::setprecision(5) << "Volume of the model '" << filename << "' is equal to " << volumeProp.Mass() << std::endl;
      }

      // Calculate the totale volume
      BRepGProp::VolumeProperties(boxWithHole, volumeProp);
      std::cout << std::setprecision(5) << "Total volume 'boxWithHole' is equal to " << volumeProp.Mass() << std::endl;
    }
    
    // Write the face (soilSurface) to a file
    STEPControl_Writer STEPwriter;
    STEPwriter.Transfer(soilSurface, STEPControl_AsIs);
    STEPwriter.Write("soilSurface.stp");
    // Write to an .stl file - First create a meshed surface
    BRepMesh_IncrementalMesh meshedSoilSurface (soilSurface, 1e-2, Standard_True);
    TopoDS_Shape soilSurfaceForExport = meshedSoilSurface.Shape();
    STLwriter.Write(soilSurfaceForExport, "soilSurface.stl");
  } else {
    // If no surface is created, we write the original geometry
    STEPControl_Writer STEPwriter;
    STEPwriter.Transfer(boxWithHole, STEPControl_AsIs);
    STEPwriter.Write("originalGeometry.stp"); 
    // To write an STL file, we first need to mesh our geometry 
    BRepMesh_IncrementalMesh meshedGeometry (boxWithHole, 1e-2, Standard_True);
    TopoDS_Shape geometryForExport = meshedGeometry.Shape();
    STLwriter.Write(geometryForExport, "originalGeometry.stl");
    // Calculate the volume of the cut geometry
    GProp_GProps volumeProperties;
    BRepGProp::VolumeProperties(boxWithHole, volumeProperties);
    std::cout << std::setprecision(5) << "Volume of the model is: " << volumeProperties.Mass() << std::endl;
    // Calculated the volume of the entire cube
    BRepGProp::VolumeProperties(box, volumeProperties);
    std::cout << std::setprecision(5) << "Volume of the original model is: " << volumeProperties.Mass() << std::endl;
  }

  return 0;
}
