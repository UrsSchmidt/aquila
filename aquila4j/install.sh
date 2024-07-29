#!/bin/bash

installpath='/usr/local/bin/aq'

jarpath="$(pwd)/aquila4j.jar"

if [[ ! -f "$jarpath" ]]; then
    echo 'Error: file not found:'
    echo "$jarpath"
    echo 'You have to build aquila4j first'
    exit 1
fi

echo '#!/bin/bash' > "$installpath"
echo '' >> "$installpath"
echo 'java -jar '"$jarpath"' "$@"' >> "$installpath"
chmod +x "$installpath"
