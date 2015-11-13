/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package lucy

import "testing"
import "os"
import "reflect"
import "strings"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestIndexerAddDoc(t *testing.T) {
	schema := createTestSchema()
	index := NewRAMFolder("")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  index,
		Schema: schema,
	})
	indexer.AddDoc(&testDoc{Content: "foo"})
	indexer.AddDoc(map[string]interface{}{"content": "foo"})
	doc := NewDoc(0)
	doc.Store("content", "foo")
	indexer.AddDoc(doc)
	indexer.Commit()
	searcher, _ := OpenIndexSearcher(index)
	if got := searcher.DocFreq("content", "foo"); got != 3 {
		t.Errorf("Didn't index all docs -- DocMax: %d", got)
	}
}

func TestIndexerAddIndex(t *testing.T) {
	var err error
	origIndex := "_test_go_indexer_add_index"
	defer os.RemoveAll(origIndex)
	schema := createTestSchema()
	indexer, err := OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  origIndex,
		Schema: schema,
	})
	if err != nil {
		t.Errorf("OpenIndexer: %v", err)
		return
	}
	err = indexer.AddDoc(&testDoc{Content: "foo"})
	if err != nil {
		t.Errorf("AddDoc: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit FS index: %v", err)
	}

	indexer, _ = OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  NewRAMFolder(""),
		Schema: schema,
	})
	err = indexer.AddIndex(origIndex)
	if err != nil {
		t.Errorf("AddIndex: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("AddIndex: %v", err)
	}
}

func TestIndexerDeletions(t *testing.T) {
	index := createTestIndex("foo", "bar", "baz", "gazi")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: index})
	err := indexer.DeleteByTerm("content", "foo")
	if err != nil {
		t.Errorf("DeleteByTerm: %v", err)
	}
	indexer.DeleteByQuery(NewTermQuery("content", "bar"))
	if err != nil {
		t.Errorf("DeleteByQuery: %v", err)
	}
	indexer.DeleteByDocID(3)
	if err != nil {
		t.Errorf("DeleteByDocID: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit: %v", err)
	}
	searcher, _ := OpenIndexSearcher(index)
	if count := searcher.GetReader().DocCount(); count != 1 {
		t.Errorf("Some deletions didn't go through (count=%d)", count)
	}
}

func TestIndexerMisc(t *testing.T) {
	var err error
	index := createTestIndex("foo", "bar", "baz")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: index})
	if _, ok := indexer.GetSchema().(Schema); !ok {
		t.Errorf("GetSchema")
	}
	if _, ok := indexer.getStockDoc().(Doc); !ok {
		t.Errorf("getStockDoc")
	}
	if _, ok := indexer.getSegWriter().(SegWriter); !ok {
		t.Errorf("getSegWriter")
	}
	indexer.AddDoc(&testDoc{Content: "gazi"})
	indexer.Optimize()
	err = indexer.PrepareCommit()
	if err != nil {
		t.Errorf("PrepareCommit: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit: %v", err)
	}
}

func TestBackgroundMergerMisc(t *testing.T) {
	var err error
	index := createTestIndex("foo", "bar", "baz")
	merger, _ := OpenBackgroundMerger(index, nil)
	merger.Optimize()
	err = merger.PrepareCommit()
	if err != nil {
		t.Errorf("PrepareCommit: %v", err)
	}
	err = merger.Commit()
	if err != nil {
		t.Errorf("Commit: %v", err)
	}
}

func TestIndexManagerAccessors(t *testing.T) {
	host := "dev.example.com"
	manager := NewIndexManager(host, nil)
	if got := manager.GetHost(); got != host {
		t.Errorf("GetHost: %v", got)
	}
	folder := NewRAMFolder("")
	manager.SetFolder(folder)
	if got := manager.GetFolder(); !reflect.DeepEqual(folder, got) {
		t.Errorf("SetFolder/GetFolder")
	}
	manager.SetWriteLockTimeout(72)
	if got := manager.GetWriteLockTimeout(); got != 72 {
		t.Errorf("set/GetWriteLockTimeout: %d", got)
	}
	manager.SetWriteLockInterval(42)
	if got := manager.GetWriteLockInterval(); got != 42 {
		t.Errorf("set/GetWriteLockInterval: %d", got)
	}
	manager.setMergeLockTimeout(73)
	if got := manager.getMergeLockTimeout(); got != 73 {
		t.Errorf("set/getMergeLockTimeout: %d", got)
	}
	manager.setMergeLockInterval(43)
	if got := manager.getMergeLockInterval(); got != 43 {
		t.Errorf("set/getMergeLockInterval: %d", got)
	}
	manager.setDeletionLockTimeout(71)
	if got := manager.getDeletionLockTimeout(); got != 71 {
		t.Errorf("set/getDeletionLockTimeout: %d", got)
	}
	manager.setDeletionLockInterval(41)
	if got := manager.getDeletionLockInterval(); got != 41 {
		t.Errorf("set/getDeletionLockInterval: %d", got)
	}
}

func TestIndexManagerLocks(t *testing.T) {
	manager := NewIndexManager("", nil)
	manager.SetFolder(NewRAMFolder(""))
	if _, ok := manager.MakeWriteLock().(Lock); !ok {
		t.Errorf("MakeWriteLock")
	}
	if _, ok := manager.makeMergeLock().(Lock); !ok {
		t.Errorf("makeMergeLock")
	}
	if _, ok := manager.makeDeletionLock().(Lock); !ok {
		t.Errorf("makeDeletionLock")
	}
	snapFile := "snapshot_4a.json"
	if _, ok := manager.makeSnapshotReadLock(snapFile).(SharedLock); !ok {
		t.Errorf("makeDeletionLock")
	}
}

func TestIndexManagerMergeData(t *testing.T) {
	var err error
	manager := NewIndexManager("", nil)
	manager.SetFolder(NewRAMFolder(""))
	err = manager.WriteMergeData(42)
	if err != nil {
		t.Errorf("WriteMergeData: %v", err)
	}
	mergeData, err := manager.ReadMergeData()
	if err != nil {
		t.Errorf("ReadMergeData: %v", err)
	}
	if got, ok := mergeData["cutoff"].(string); !ok || got != "42" {
		t.Errorf("ReadMergeData: %v", got)
	}
	err = manager.RemoveMergeData()
	if err != nil {
		t.Errorf("RemoveMergeData: %v", err)
	}
}

func TestIndexManagerMisc(t *testing.T) {
	manager := NewIndexManager("", nil)
	manager.SetFolder(NewRAMFolder(""))
	if got, err := manager.MakeSnapshotFilename(); !strings.Contains(got, "snapshot") || err != nil {
		t.Errorf("MakeSnapshotFilename: %s, %v", got, err)
	}
	snapshot := NewSnapshot()
	snapshot.AddEntry("seg_4")
	snapshot.AddEntry("seg_5")
	if got := manager.highestSegNum(snapshot); got != 5 {
		t.Errorf("highestSegNum: %d", got)
	}
}

func TestIndexManagerRecycle(t *testing.T) {
	index := createTestIndex("foo", "bar", "baz")
	manager := NewIndexManager("", nil)
	manager.SetFolder(index)
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: index})
	searcher, _ := OpenIndexSearcher(index)
	reader := searcher.GetReader().(PolyReader)
	delWriter := indexer.getSegWriter().getDelWriter()
	segReaders, err := manager.Recycle(reader, delWriter, 0, true)
	if err != nil || len(segReaders) != 1 {
		t.Errorf("Recycle: (%d SegReaders) %v", len(segReaders), err)
	}
}

func TestTermInfoMisc(t *testing.T) {
	tinfo := NewTermInfo(1000)
	if got := tinfo.GetDocFreq(); got != 1000 {
		t.Errorf("GetDocFreq: %d", got)
	}
	tinfo.SetDocFreq(1001)
	if got := tinfo.GetDocFreq(); got != 1001 {
		t.Errorf("Set/GetDocFreq: %d", got)
	}
	tinfo.SetLexFilePos(1002)
	if got := tinfo.GetLexFilePos(); got != 1002 {
		t.Errorf("Set/GetLexFilePos: %d", got)
	}
	tinfo.SetPostFilePos(1003)
	if got := tinfo.GetPostFilePos(); got != 1003 {
		t.Errorf("Set/GetPostFilePos: %d", got)
	}
	tinfo.SetSkipFilePos(1002)
	if got := tinfo.GetSkipFilePos(); got != 1002 {
		t.Errorf("Set/GetSkipFilePos: %d", got)
	}
	other := NewTermInfo(42)
	other.Mimic(tinfo)
	if got := other.GetDocFreq(); got != tinfo.GetDocFreq() {
		t.Errorf("Mimic: (%d != %d)", got, tinfo.GetDocFreq())
	}
	other = tinfo.Clone().(TermInfo)
	if got := other.GetDocFreq(); got != tinfo.GetDocFreq() {
		t.Errorf("Clone: (%d != %d)", got, tinfo.GetDocFreq())
	}
	tinfo.reset()
	if got := tinfo.GetDocFreq(); got != 0 {
		t.Errorf("Reset: expected 0, got %d", got)
	}
}

func TestBitVecDelDocsMisc(t *testing.T) {
	folder := NewRAMFolder("")
	out, _ := folder.OpenOut("bits")
	out.WriteU32(0xDEADBEEF)
	out.Close()
	bv := NewBitVecDelDocs(folder, "bits")
	if !bv.Get(31) {
		t.Errorf("Get returned false")
	}
}

func TestTermVectorMisc(t *testing.T) {

	positions := []int32{0, 3}
	startOffsets := []int32{0, 20}
	endOffsets := []int32{2, 22}
	tv := NewTermVector("content", "red yellow green red blue", positions, startOffsets, endOffsets)
	if got := tv.GetPositions(); !reflect.DeepEqual(got, positions) {
		t.Errorf("GetPositions: %v", got)
	}
	if got := tv.GetStartOffsets(); !reflect.DeepEqual(got, startOffsets) {
		t.Errorf("GetStartOffsets: %v", got)
	}
	if got := tv.GetEndOffsets(); !reflect.DeepEqual(got, endOffsets) {
		t.Errorf("GetEndOffsets: %v", got)
	}

	folder := NewRAMFolder("")
	out, _ := folder.OpenOut("dump")
	tv.serialize(out)
	out.Close()
	in, _ := folder.OpenIn("dump")
	dupe := clownfish.GetClass(tv).MakeObj().(TermVector).deserialize(in)
	if !tv.Equals(dupe) {
		t.Errorf("Unsuccessful serialization round trip")
	}
}

func TestDocVectorMisc(t *testing.T) {
	schema := NewSchema()
	spec := NewFullTextType(NewStandardTokenizer())
	spec.SetHighlightable(true)
	schema.SpecField("content", spec)
	folder := NewRAMFolder("")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: folder, Schema: schema, Create: true})
	indexer.AddDoc(&testDoc{Content: "foo bar baz"})
	indexer.Commit()
	searcher, _ := OpenIndexSearcher(folder)
	dv, _ := searcher.fetchDocVec(1)
	fieldBuf := dv.fieldBuf("content");
	if fieldBuf == nil {
		t.Errorf("fieldBuf returned nil")
	}
	dv.addFieldBuf("content", fieldBuf)
	if got := dv.termVector("content", "bar"); got == nil {
		t.Errorf("termVector returned nil")
	}

	out, _ := folder.OpenOut("dump")
	dv.serialize(out)
	out.Close()
	in, _ := folder.OpenIn("dump")
	dupe := clownfish.GetClass(dv).MakeObj().(DocVector).deserialize(in)
	in.Close()
	if _, ok := dupe.(DocVector); !ok {
		t.Errorf("serialize/deserialize")
	}
}

func TestSnapshotMisc(t *testing.T) {
	var err error
	snapshot := NewSnapshot()
	snapshot.AddEntry("foo")
	snapshot.AddEntry("bar")
	snapshot.DeleteEntry("bar")
	if got := snapshot.NumEntries(); got != 1 {
		t.Errorf("Add/DeleteEntry, NumEntries: %d", got)
	}
	if got := snapshot.List(); !reflect.DeepEqual(got, []string{"foo"}) {
		t.Errorf("List: %v", got)
	}
	folder := NewRAMFolder("")
	err = snapshot.WriteFile(folder, "")
	if err != nil {
		t.Errorf("WriteFile: %v", err)
	}
	other := NewSnapshot()
	_, err = other.ReadFile(folder, "")
	if err != nil {
		t.Errorf("ReadFile: %v", err)
	}

	path := "snapshot_4.json"
	snapshot.SetPath(path)
	if got := snapshot.GetPath(); got != path {
		t.Errorf("SetPath/GetPath: %v", path)
	}
}

func TestSortCacheMisc(t *testing.T) {
	var err error

	schema := NewSchema()
	spec := NewFullTextType(NewStandardTokenizer())
	spec.SetSortable(true)
	schema.SpecField("content", spec)
	folder := NewRAMFolder("")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: folder, Schema: schema, Create: true})
	indexer.AddDoc(&testDoc{Content: "foo"})
	indexer.AddDoc(&testDoc{Content: "bar"})
	indexer.AddDoc(&testDoc{Content: "baz"})
	indexer.AddDoc(make(map[string]interface{}))
	indexer.Commit()

	searcher, _ := OpenIndexSearcher(folder)
	segReaders := searcher.GetReader().SegReaders()
	sortReader := segReaders[0].(SegReader).Obtain("Lucy::Index::SortReader").(SortReader)
	sortCache := sortReader.fetchSortCache("content")

	if card := sortCache.GetCardinality(); card != 4 {
		t.Errorf("GetCardinality: %d", card)
	}
	if width := sortCache.GetOrdWidth(); width != 2 {
		t.Errorf("GetOrdWidth: %d", width)
	}

	lowest, err := sortCache.Value(0)
	if _, ok := lowest.(string); err != nil || !ok || lowest != "bar" {
		t.Errorf("Value: %v", err)
	}
	if ord, err := sortCache.Ordinal(1); err != nil || ord != 2 { // "foo" is ordinal 2
		t.Errorf("Ordinal: %d, %v", ord, err)
	}
	if nullOrd := sortCache.GetNullOrd(); nullOrd != 3 {
		t.Errorf("GetNullOrd: %d", nullOrd)
	}

	if ord, err := sortCache.Find("foo"); err != nil || ord != 2 {
		t.Errorf("Find: %d, %v", ord, err)
	}

	if sortCache.getNativeOrds() {
		t.Errorf("recent index shouldn't have native ords")
	}
}

func TestSimilarityMisc(t *testing.T) {
	sim := NewSimilarity()
	if _, ok := sim.makePosting().(Posting); !ok {
		t.Errorf("makePosting")
	}
	if got := sim.tF(4.0); got != 2.0 {
		t.Errorf("tF: %f", got)
	}
	if got := sim.iDF(40, 1000); got <= 0 {
		t.Errorf("iDF: %f", got)
	}
	if got := sim.coord(3, 4); got <= 0 {
		t.Errorf("coord: %f", got)
	}
	if got := sim.LengthNorm(4); got != 0.5 {
		t.Errorf("LengthNorm: %f", got)
	}
	if got := sim.queryNorm(4); got != 0.5 {
		t.Errorf("queryNorm: %f", got)
	}
	if got := sim.encodeNorm(sim.decodeNorm(42)); got != 42 {
		t.Errorf("encode/decodeNorm: %d", got)
	}
}

func TestSimilarityRoundTrip(t *testing.T) {
	sim := NewSimilarity()
	dupe := sim.load(sim.dump())
	if !sim.Equals(dupe) {
		t.Errorf("Dump/Load round-trip")
	}
	folder := NewRAMFolder("")
	out, _ := folder.OpenOut("dump")
	sim.serialize(out)
	out.Close()
	in, _ := folder.OpenIn("dump")
	dupe = clownfish.GetClass(sim).MakeObj().(Similarity).deserialize(in)
	if !sim.Equals(dupe) {
		t.Errorf("serialize/deserialize round-trip")
	}
}

func TestSegmentMisc(t *testing.T) {
	var err error

	seg := NewSegment(4)
	if name := seg.GetName(); name != "seg_4" {
		t.Errorf("GetName: %s", name)
	}
	if num := seg.GetNumber(); num != 4 {
		t.Errorf("GetNumber: %d", num)
	}

	if num := seg.AddField("field1"); num != 1 {
		t.Errorf("AddField: %d", num)
	}
	if field := seg.FieldName(1); field != "field1" {
		t.Errorf("FieldName: %v", field)
	}
	if num := seg.FieldNum("field1"); num != 1 {
		t.Errorf("FieldNum: %d", num)
	}

	metadata := map[string]interface{}{
		"foo": "1",
		"bar": "2",
	}
	seg.StoreMetadata("mycomponent", metadata)
	if got := seg.FetchMetadata("mycomponent"); !reflect.DeepEqual(got, metadata) {
		t.Errorf("Store/FetchMetadata: %v", got)
	}
	if got := seg.getMetadata(); got["mycomponent"] == nil {
		t.Errorf("%v", got)
	}

	seg.SetCount(42)
	seg.incrementCount(5)
	if got := seg.GetCount(); got != 47 {
		t.Errorf("SetCount/GetCount: %d", got)
	}

	folder := NewRAMFolder("")
	folder.MkDir("seg_4")
	err = seg.WriteFile(folder)
	if err != nil {
		t.Errorf("WriteFile: %v", err)
	}
	dupe := NewSegment(4)
	dupe.ReadFile(folder)
	if err != nil {
		t.Errorf("WriteFile: %v", err)
	}

	other := NewSegment(5)
	if got := seg.CompareTo(other); got >= 0 {
		t.Errorf("CompareTo (seg 4 vs seg 5): %d", got)
	}
}

func TestFilePurgerMisc(t *testing.T) {
	folder := NewRAMFolder("")
	oldSnapshot := NewSnapshot()
	oldSnapshot.AddEntry("foo")
	out, _ := folder.OpenOut("foo")
	out.Close()
	oldSnapshot.WriteFile(folder, "")

	snapshot := NewSnapshot()
	snapshot.WriteFile(folder, "")
	purger := NewFilePurger(folder, snapshot, nil)
	purger.purge()
	if folder.exists("foo") {
		t.Errorf("Failed to purge file")
	}
}

func TestInvEntryMisc(t *testing.T) {
	entry := NewInverterEntry(nil, "content", 1)
	other := NewInverterEntry(nil, "title", 2)
	if got := entry.CompareTo(other); got >= 0 {
		t.Errorf("CompareTo compares field numbers: %d", got)
	}
}

func TestInverterMisc(t *testing.T) {

	schema := NewSchema()
	spec := NewFullTextType(NewStandardTokenizer())
	schema.SpecField("title", spec)
	schema.SpecField("content", spec)
	seg := NewSegment(4)
	seg.AddField("title")
	seg.AddField("content")

	inverter := NewInverter(schema, seg)
	inverter.SetBoost(2.0)
	if got := inverter.GetBoost(); got != 2.0 {
		t.Errorf("Set/GetBoost: %f", got)
	}
	if got := inverter.GetDoc(); got != nil {
		t.Errorf("GetDoc should return nil when not yet set")
	}
	inverter.Clear()

	doc := NewDoc(42)
	doc.Store("title", "Foo")
	doc.Store("content", "foo foo foo foo foo")
	inverter.SetDoc(doc)
	if docID := inverter.GetDoc().GetDocID(); docID != 42 {
		t.Errorf("Set/GetDoc: %d", docID)
	}
	inverter.Clear()
	// Can't test AddField.
	// inverter.AddField(entry)
	inverter.InvertDoc(doc)


	numFields := inverter.Iterate()
	if numFields != 2 {
		t.Errorf("Iterate: %d", numFields)
	}
	fieldNum := inverter.Next()
	if fieldNum != 1 {
		t.Errorf("Next: %d", fieldNum)
	}
	if got := inverter.GetFieldName(); got != "title" {
		t.Errorf("GetFieldName during first iter: %s", got)
	}
	if got := inverter.GetValue(); got != "Foo" {
		t.Errorf("GetValue during first iter: %v", got)
	}
	if got, ok := inverter.GetType().(FullTextType); !ok {
		t.Errorf("GetType: %T", got)
	}
	if got, ok := inverter.GetAnalyzer().(StandardTokenizer); !ok {
		t.Errorf("GetType: %T", got)
	}
	if got, ok := inverter.GetSimilarity().(Similarity); !ok {
		t.Errorf("GetSimilarity: %T", got)
	}
	if got, ok := inverter.GetInversion().(Inversion); !ok {
		t.Errorf("GetInversion: %T", got)
	}

	if nextFieldNum := inverter.Next(); nextFieldNum != 2 {
		t.Errorf("Next second iter: %d", nextFieldNum)
	}

	if done := inverter.Next(); done != 0 {
		t.Errorf("Next should return 0 when exhausted: %d", done)
	}
	if got := inverter.GetFieldName(); got != "" {
		t.Errorf("GetFieldName after iterator exhausted: %s", got)
	}
	if got := inverter.GetValue(); got != nil {
		t.Errorf("GetValue after iterator exhausted: %v", got)
	}
	if got := inverter.GetType(); got != nil {
		t.Errorf("GetType after iterator exhausted: %v", got)
	}
	if got := inverter.GetAnalyzer(); got != nil {
		t.Errorf("GetAnalyzer after iterator exhausted: %v", got)
	}
	if got := inverter.GetSimilarity(); got != nil {
		t.Errorf("GetSimilarity after iterator exhausted: %v", got)
	}
	if got := inverter.GetInversion(); got != nil {
		t.Errorf("GetInversion after iterator exhausted: %v", got)
	}
}
