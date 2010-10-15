#define C_KINO_INDEXREADER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/IndexManager.h"
#include "KinoSearch/Index/PolyReader.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/Matcher.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Store/Lock.h"

IndexReader*
IxReader_open(Obj *index, Snapshot *snapshot, IndexManager *manager)
{
    return IxReader_do_open(NULL, index, snapshot, manager);
}

IndexReader*
IxReader_do_open(IndexReader *temp_self, Obj *index, Snapshot *snapshot, 
                 IndexManager *manager)
{
    PolyReader *polyreader = PolyReader_open(index, snapshot, manager);
    if (!VA_Get_Size(PolyReader_Get_Seg_Readers(polyreader))) {
        THROW(ERR, "Index doesn't seem to contain any data");
    }
    DECREF(temp_self);
    return (IndexReader*)polyreader;
}

IndexReader*
IxReader_init(IndexReader *self, Schema *schema, Folder *folder, 
              Snapshot *snapshot, VArray *segments, int32_t seg_tick, 
              IndexManager *manager)
{
    snapshot = snapshot ? (Snapshot*)INCREF(snapshot) : Snapshot_new();
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
        seg_tick);
    DECREF(snapshot);
    self->components     = Hash_new(0);
    self->read_lock      = NULL;
    self->deletion_lock  = NULL;
    if (manager) {
        self->manager = (IndexManager*)INCREF(manager);
        IxManager_Set_Folder(self->manager, self->folder);
    }
    else {
        self->manager = NULL;
    }
    return self;
}

void
IxReader_close(IndexReader *self)
{
    if (self->components) {
        CharBuf *key;
        DataReader *component;
        Hash_Iterate(self->components);
        while (
            Hash_Next(self->components, (Obj**)&key, (Obj**)&component)
        ) {
            if (Obj_Is_A((Obj*)component, DATAREADER)) { 
                DataReader_Close(component); 
            }
        }
        Hash_Clear(self->components);
    }
    if (self->read_lock) {
        Lock_Release(self->read_lock);
        DECREF(self->read_lock);
        self->read_lock = NULL;
    }
}

void
IxReader_destroy(IndexReader *self)
{
    DECREF(self->components);
    if (self->read_lock) {
        Lock_Release(self->read_lock);
        DECREF(self->read_lock);
    }
    DECREF(self->manager);
    DECREF(self->deletion_lock);
    SUPER_DESTROY(self, INDEXREADER);
}

Hash*
IxReader_get_components(IndexReader *self) 
    { return self->components; }

DataReader*
IxReader_obtain(IndexReader *self, const CharBuf *api)
{
    DataReader *component 
        = (DataReader*)Hash_Fetch(self->components, (Obj*)api);
    if (!component) {
        THROW(ERR, "No component registered for '%o'", api);
    }
    return component;
}

DataReader*
IxReader_fetch(IndexReader *self, const CharBuf *api)
{
    return (DataReader*)Hash_Fetch(self->components, (Obj*)api);
}


