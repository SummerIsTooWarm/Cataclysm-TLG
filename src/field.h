#pragma once
#ifndef CATA_SRC_FIELD_H
#define CATA_SRC_FIELD_H

#include <map>
#include <string>
#include <vector>

#include "calendar.h"
#include "cata_lazy.h"
#include "color.h"
#include "enums.h"
#include "field_type.h"
#include "type_id.h"

/**
 * An active or passive effect existing on a tile.
 * Each effect can vary in intensity and age (usually used as a time to live).
 */
class field_entry
{
    public:
        field_entry() : type( fd_null.id_or( INVALID_FIELD_TYPE_ID ) ), intensity( 1 ), age( 0_turns ),
            is_alive( false ) { }
        field_entry( const field_type_id &t, const int i, const time_duration &a ) : type( t ),
            intensity( i ), age( a ), is_alive( true ) { }

        nc_color color() const;

        std::string symbol() const;

        // return intensity level object corresponding to the current field intensity
        const field_intensity_level &get_intensity_level() const;

        //Returns the field_type_id of the current field entry.
        field_type_id get_field_type() const;

        // Allows you to modify the field_type_id of the current field entry.
        // This probably shouldn't be called outside of field::replace_field, as it
        // breaks the field drawing code and field lookup
        field_type_id set_field_type( const field_type_id &new_type );

        // Returns the maximum intensity of the current field entry.
        int get_max_field_intensity() const;
        // Returns the current intensity of the current field entry.
        int get_field_intensity() const;
        // Allows you to modify the intensity of the current field entry.
        int set_field_intensity( int new_intensity );
        void mod_field_intensity( int mod );

        /// @returns @ref age.
        time_duration get_field_age() const;
        /// Sets @ref age to the given value.
        /// @returns New value of @ref age.
        time_duration set_field_age( const time_duration &new_age );
        /// Adds given value to @ref age.
        /// @returns New value of @ref age.
        time_duration mod_field_age( const time_duration &mod_age ) {
            return set_field_age( get_field_age() + mod_age );
        }
        fake_spell spell_data;
        bool is_dangerous() const;
        bool is_mopsafe() const;

        //Returns the display name of the current field given its current intensity.
        //IE: light smoke, smoke, heavy smoke
        std::string name() const {
            return type.obj().get_name( intensity - 1 );
        }

        //Returns true if this is an active field, false if it should be removed.
        bool is_field_alive() const {
            return intensity > 0 && is_alive;
        }

        bool gas_can_spread() const {
            return is_field_alive() && type.obj().phase == phase_id::GAS && type.obj().percent_spread > 0;
        }

        void initialize_decay();
        void do_decay();

        std::vector<field_effect> field_effects() const;

    private:
        // The field identifier.
        field_type_id type;
        // The intensity (higher is stronger), of the field entry.
        int intensity;
        // The age, of the field effect. 0 is permanent.
        time_duration age;
        // The time when the field will decay, initialized to 0.
        time_point decay_time;
        // True if this is an active field, false if it should be destroyed next check.
        bool is_alive;
};

/**
 * A variable sized collection of field entries on a given map square.
 * It contains one (at most) entry of each field type (e. g. one smoke entry and one
 * fire entry, but not two fire entries).
 * Use @ref find_field to get the field entry of a specific type, or iterate over
 * all entries via @ref begin and @ref end (allows range based iteration).
 * There is @ref displayed_field_type to specific which field should be drawn on the map.
*/
class field
{
    public:
        field();

        /**
         * Returns a field entry corresponding to the field_type_id parameter passed in.
         * If no fields are found then nullptr is returned.
         * @param alive_only if true, returns non-null result only if field is alive
         */
        field_entry *find_field( const field_type_id &field_type_to_find, bool alive_only = true );
        /**
         * Returns a field entry corresponding to the field_type_id parameter passed in.
         * If no fields are found then nullptr is returned.
         * @param alive_only if true, returns non-null result only if field is alive
         */
        const field_entry *find_field( const field_type_id &field_type_to_find,
                                       bool alive_only = true ) const;

        /**
         * Inserts the given field_type_id into the field list for a given tile if it does not already exist.
         * If you wish to modify an already existing field use find_field and modify the result.
         * Intensity defaults to 1, and age to 0 (permanent) if not specified.
         * The intensity is added to an existing field entry, but the age is only used for newly added entries.
         * @return false if the field_type_id already exists, true otherwise.
         */
        bool add_field( const field_type_id &field_type_to_add, int new_intensity = 1,
                        const time_duration &new_age = 0_turns );

        /**
         * Removes the field entry with a type equal to the field_type_id parameter.
         * Make sure to decrement the field counter in the submap if (and only if) the
         * function returns true.
         * @return True if the field was removed, false if it did not exist in the first place.
         */
        bool remove_field( const field_type_id &field_to_remove );
        /**
         * Make sure to decrement the field counter in the submap.
         * Removes the field entry, the iterator must point into @ref _field_type_list and must be valid.
         */
        void remove_field( std::map<field_type_id, field_entry>::iterator );

        /**
         * Removes all fields.
         * Don't forget to update field count in submap!
         */
        void clear();

        // Returns the number of fields existing on the current tile.
        unsigned int field_count() const;

        /**
         * Returns field type that should be drawn.
         */
        field_type_id displayed_field_type() const;

        /**
         * Returns the intensity of the drawn tile
         */
        int displayed_intensity() const;

        description_affix displayed_description_affix() const;

        //Returns the vector iterator to begin searching through the list.
        std::map<field_type_id, field_entry>::iterator begin();
        std::map<field_type_id, field_entry>::const_iterator begin() const;

        //Returns the vector iterator to end searching through the list.
        std::map<field_type_id, field_entry>::iterator end();
        std::map<field_type_id, field_entry>::const_iterator end() const;

        /**
         * Returns the total move cost from all fields.
         */
        int total_move_cost() const;

    private:
        // A pointer lookup table of all field effects on the current tile.
        lazy<std::map<field_type_id, field_entry>> _field_type_list;
        //_displayed_field_type currently is equal to the last field added to the square. You can modify this behavior in the class functions if you wish.
        field_type_id _displayed_field_type;
};

#endif // CATA_SRC_FIELD_H
