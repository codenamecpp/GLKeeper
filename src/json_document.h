#pragma once

// internals
struct cJSON;

namespace cxx
{
    // element of configuration document
    class json_document_node
    {
        friend class json_document;

    public:
        json_document_node();
        ~json_document_node();

        // get next / previous sibling element
        json_document_node next_sibling() const;
        json_document_node prev_sibling() const;

        // get first child element of object or array element
        json_document_node first_child() const;

        // get child node element by name
        // @param name: Node name
        json_document_node operator [] (const char* name) const;

        // get array element by index if array
        // get child element by index if object
        // @param elementIndex: Array element index
        json_document_node operator [] (int elementIndex) const;

        // get number of elements in array
        // get number of child elements if object
        int get_elements_count() const;

        // test whether child node element is exists
        // @param name: Name
        bool is_child_exists(const char* name) const;

        // determine type of element value
        bool is_string_element() const;
        bool is_number_element() const;
        bool is_boolean_element() const;
        bool is_array_element() const;
        bool is_object_element() const;

        // get element value
        const char* get_value_string() const;
        bool get_value_boolean() const;
        int get_value_integer() const;
        float get_value_float() const;

        // get name of element
        const char* get_element_name() const;

        // operators
        inline bool operator == (const json_document_node& rhs) const { return mJsonElement && rhs.mJsonElement == mJsonElement; }
        inline bool operator != (const json_document_node& rhs) const { return !mJsonElement || rhs.mJsonElement != mJsonElement; }
        inline operator bool () const { return mJsonElement != nullptr; }
        
    private:
        json_document_node(cJSON* jsonElementImplementation);

    private:
        cJSON* mJsonElement;
    };

    // json document
    class json_document: public noncopyable
    {
    public:
        // @param content: Content string
        json_document(const char* content);
        json_document();
        ~json_document();

        // parse json document from string
        // @param content: Content string
        bool parse_document(const char* content);
        void close_document();

        // Get root json object of document
        json_document_node get_root_node() const;

        // test whether json document is loaded
        bool is_loaded() const;

    private:
        cJSON* mJsonElement;
    };

} // namespace cxx