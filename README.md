kayak
=====

wkcc kayak software

STATUS::

---------------------------------

schema/schema.sql # Hopefully a full database schema, DONE

# This have been modified for W3's validator and Chrome's audit
web.templates/levels.head # Included by mkMainPage.py
web.templates/levels.tail # Included by mkMainpage.py

css/levels.css # Include by index.html

---------------------------------

-- Database conversion from old to new, DONE

python/rebuild

-- This calls mysql installing the new database schema  
-- old2new_info.py populates the basic non-data tables from the old database
-- old2new_data.py populates the data table from the old database
-- fixParameters patches the new database with a few fixes

---------------------------------

-- build index.html, DONE

mkMainPage.py # Builds index.html with the list of states

-- This has been run through Chrome's audit and W3's validator

---------------------------------

LEFT TO DO!!!!!!!!!!!!!!!!

---------------------------------

mkDescription
fetcher
calcRating
merger
calculator
maps
builder

Installation makefile
