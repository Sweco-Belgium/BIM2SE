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

// BRepBuilderAPI package - part of the TKTopAlgo toolit
#include <BRepBuilderAPI_MakeFace.hxx> // Build a face from a surface

// BRepAlgoAPI package - part of TKBO toolkit
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Splitter.hxx>

// ShapeBuild_ReShape - part of the TKShHealing toolkit
#include <ShapeBuild_ReShape.hxx>

// Writa a STL file
#include <StlAPI_Writer.hxx>

// Write a STEPfile
#include <STEPControl_Writer.hxx>

// Include GProp package - calculate global properties
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

// Type definitions
typedef NCollection_Array2<gp_Pnt> ListOfPoints; // the same as TColgp_Array2OfPnt

// Include goodbye if set
#ifdef USE_SANDBOX
#   include <sandbox.h>
#endif

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
      std::cout << "Slicer activated... (see FreeCAD Part Slice)" << std::endl;

      TopoDS_Shape test123 = splitter.Shape();
      
      // Write a STEP file
      STEPwriter.Transfer(test123, STEPControl_AsIs);
      STEPwriter.Write("test123.stp");

      // Write a STL file
      BRepMesh_IncrementalMesh meshedTest123 (test123, 1e-2, Standard_True);
      TopoDS_Shape test123Export = meshedTest123.Shape();
      STLwriter.Write(test123Export, "test123.stl");

      // Retrieve the geometry
      // Inspiration : https://github.com/trelau/AFEM/commit/08fbad73a9b21dae2ed5c44d6971c98af580157a
      TopTools_ListOfShape slices = splitter.Modified(boxWithHole);

      if(not slices.IsEmpty())
      {
        std::cout << "Geometry has been sliced!!" << std::endl;

        if(splitter.HasModified())
          std::cout << "Shapes have been modified" << std::endl;
        if(splitter.HasGenerated())
          std::cout << "Shapes have been generated" << std::endl;
        if(splitter.HasDeleted())
          std::cout << "Shapes have been deleted" << std::endl;

        TopoDS_Shape slice1 = slices.First();

        // Slice 1 - Export
        BRepMesh_IncrementalMesh meshedSlice1 (slice1, 1e-2, Standard_True);
        TopoDS_Shape Slice1Export = meshedSlice1.Shape();
        STLwriter.Write(Slice1Export, "slice1.stl");
      }
    }
    
    // Write the face (soilSurface) to a file
    STEPwriter.Transfer(soilSurface, STEPControl_AsIs);
    STEPwriter.Write("soilSurface.stp");
    // Write to an .stl file - First create a meshed surface
    BRepMesh_IncrementalMesh meshedSoilSurface (soilSurface, 1e-2, Standard_True);
    TopoDS_Shape soilSurfaceForExport = meshedSoilSurface.Shape();
    STLwriter.Write(soilSurfaceForExport, "soilSurface.stl");
  } else {
    // If no surface is created, we write the original geometry
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
