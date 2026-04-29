use v5.42;

my $command = `
gh api graphql -f query='{
  viewer {
    projectV2(number:3) {
      number
      title
      fields(first: 100) {
        nodes {
          ... on ProjectV2Field { dataType name }
          ... on ProjectV2IterationField { dataType name }
          ... on ProjectV2SingleSelectField { dataType name options { color description name } }
        }
      }
      items(first: 100) {
        nodes {
          id
          fieldValues(first: 100) {
            nodes {
              __typename
              ... on ProjectV2ItemFieldTextValue { text }
              ... on ProjectV2ItemFieldSingleSelectValue {
                color
                name
                item {
                  type
                  id
                }
              }
            }
          }
        }
      }
    }
  }
}'
`;

say $command;
