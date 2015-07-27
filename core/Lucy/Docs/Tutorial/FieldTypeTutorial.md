# Specify per-field properties and behaviors.

The Schema we used in the last chapter specifies three fields: 

``` c
    FullTextType *type = FullTextType_new((Analyzer*)analyzer);

    {
        String *field_str = Str_newf("title");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("content");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("url");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

```

``` perl
my $type = Lucy::Plan::FullTextType->new(
    analyzer => $easyanalyzer,
);
$schema->spec_field( name => 'title',   type => $type );
$schema->spec_field( name => 'content', type => $type );
$schema->spec_field( name => 'url',     type => $type );
```

Since they are all defined as "full text" fields, they are all searchable --
including the `url` field, a dubious choice.  Some URLs contain meaningful
information, but these don't, really:

    http://example.com/us_constitution/amend1.txt

We may as well not bother indexing the URL content.  To achieve that we need
to assign the `url` field to a different FieldType.  

## StringType

Instead of FullTextType, we'll use a
[](cfish:lucy.StringType), which doesn't use an
Analyzer to break up text into individual fields.  Furthermore, we'll mark
this StringType as unindexed, so that its content won't be searchable at all.

``` c
    {
        String *field_str = Str_newf("url");
        StringType *type = StringType_new();
        StringType_Set_Indexed(type, false);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }
```

``` perl
my $url_type = Lucy::Plan::StringType->new( indexed => 0 );
$schema->spec_field( name => 'url', type => $url_type );
```

To observe the change in behavior, try searching for `us_constitution` both
before and after changing the Schema and re-indexing.

## Toggling 'stored'

For a taste of other FieldType possibilities, try turning off `stored` for
one or more fields.

``` c
    FullTextType *content_type = FullTextType_new((Analyzer*)analyzer);
    FullTextType_Set_Stored(content_type, false);
```

``` perl
my $content_type = Lucy::Plan::FullTextType->new(
    analyzer => $easyanalyzer,
    stored   => 0,
);
```

Turning off `stored` for either `title` or `url` mangles our results page,
but since we're not displaying `content`, turning it off for `content` has
no effect -- except on index size.

## Analyzers up next

Analyzers play a crucial role in the behavior of FullTextType fields.  In our
next tutorial chapter, [](cfish:AnalysisTutorial), we'll see how
changing up the Analyzer changes search results.


