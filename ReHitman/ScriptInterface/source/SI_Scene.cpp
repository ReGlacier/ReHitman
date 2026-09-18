#include <SI/SI_Scene.h>

namespace Glacier
{
    void Scene__Setobjectposition(ZREF rObject, float x, float y, float z)
    {
        // TODO: Finish me
    }

    void Scene__Setobjectpositionv3(ZREF rObject, v3 position)
    {
        // TODO: Finish me
    }

    v3 Scene__Getobjectposition(ZREF rObject)
    {
        // TODO: Finish me
        return {};
    }

    int Scene__Getgeomsinsidebox(ZREF rBox, ZREF* outGeoms, int maxGeoms, bool includeInactive, bool includeChildren)
    {
        // TODO: Finish me
        return 0;
    }

    int Scene__Getactorsinbox(ZREF rBox, ZREF* outActors, int maxActors)
    {
        // TODO: Finish me
        return 0;
    }

    bool Scene__Isbox(ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }

    bool Scene__Isposinsidebox(ZREF rBox, v3 position)
    {
        // TODO: Finish me
        return false;
    }

    v3 Scene__Getposinbox(ZREF rBox)
    {
        // TODO: Finish me
        return {};
    }

    void Scene__Setscale(ZREF rObject, float x, float y, float z)
    {
        // TODO: Finish me
    }

    bool Scene__Objecthascontroller(ZREF rObject, const char* controllerName)
    {
        // TODO: Finish me
        return false;
    }

    ZREF Scene__Getcontrolleronobject(ZREF rObject, const char* controllerName)
    {
        // TODO: Finish me
        return 0;
    }

    float Scene__Getdistancebetweenobjects(ZREF firstObject, ZREF secondObject)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Scene__Getdistbetweenobjectandpos(ZREF rObject, v3 position)
    {
        // TODO: Finish me
        return 0.0f;
    }

    float Scene__Getdistbetweenobjectandpos2D(ZREF rObject, v3 position)
    {
        // TODO: Finish me
        return 0.0f;
    }

    int Scene__Getnumofaccessdenies(ZREF rObject, int accessType)
    {
        // TODO: Finish me
        return 0;
    }

    void Scene__Makeinactive(ZREF rObject)
    {
        // TODO: Finish me
    }

    void Scene__Makeactive(ZREF rObject)
    {
        // TODO: Finish me
    }

    int Scene__Getdoorstatus(ZREF rObject)
    {
        // TODO: Finish me
        return 0;
    }

    ZREF Scene__Getparent(ZREF rObject)
    {
        // TODO: Finish me
        return 0;
    }

    bool Scene__Isshattered(ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }

    bool Scene__Isground(ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }

    bool Scene__Setnoiselevel(ZREF rObject, int level)
    {
        // TODO: Finish me
        return false;
    }

    int Scene__Getnoiselevel(ZREF rObject)
    {
        // TODO: Finish me
        return -1;
    }

    bool Scene__Getisinoutsidelocation(ZREF rObject)
    {
        // TODO: Finish me
        return false;
    }
}
